"""Check staged changes without rewriting sources or the Git index."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
SCOPES = ("core/include/", "core/src/", "core/tests/", "core/sandbox/",
          "editor/include/", "editor/src/", "editor/tests/")
SOURCES = {".c", ".cc", ".cpp", ".cxx"}
HEADERS = {".h", ".hpp", ".hxx", ".inl"}
VENDORED = ("core/src/Graphics/internal/", "core/include/robot2D/Graphics/internal/")


class CheckError(RuntimeError):
    pass


def own_cpp(path):
    return (path.startswith(SCOPES) and not path.startswith(VENDORED)
            and Path(path).suffix.lower() in SOURCES | HEADERS)


def build_input(path):
    return (own_cpp(path) or path.endswith("CMakeLists.txt") or path.endswith(".cmake")
            or path in {".clang-tidy", "uncrustify.cfg", "scripts/run_clang_tidy.ps1"})


def execute(command, *, capture=False):
    if not capture:
        print("pre-commit: " + subprocess.list2cmdline([str(arg) for arg in command]), flush=True)
    result = subprocess.run([str(arg) for arg in command], cwd=ROOT, capture_output=capture,
                            text=True, encoding="utf-8", errors="replace")
    if result.returncode:
        raise CheckError(f"Command failed ({result.returncode}): {command[0]} "
                         + (result.stderr.strip() if capture else ""))
    return result.stdout if capture else ""


def git_paths(*args):
    return [name for name in execute(["git", *args, "-z"], capture=True).split("\0") if name]


def option(name, default):
    value = subprocess.run(["git", "config", "--get", f"robot2d.hooks.{name}"], cwd=ROOT,
                           capture_output=True, text=True, encoding="utf-8", errors="replace")
    if value.returncode not in (0, 1):
        raise CheckError("Cannot read hook configuration")
    return value.stdout.strip() if value.returncode == 0 else default


def executable(value):
    path = shutil.which(value)
    if path:
        return path
    if Path(value).is_file():
        return str(Path(value).resolve())
    raise CheckError(f"Missing tool: {value}. See docs/development-tools.md.")


def normal(path):
    return os.path.normcase(os.path.normpath(str(path)))


def source_path(entry):
    path = Path(entry["file"])
    return path if path.is_absolute() else Path(entry["directory"]) / path


def own_entries(entries):
    selected = []
    for entry in entries:
        path = source_path(entry).resolve()
        try:
            relative = path.relative_to(ROOT).as_posix()
        except ValueError:
            continue
        if own_cpp(relative):
            selected.append(entry)
    return selected


def affected_units(changed, entries, dependencies, root):
    """Map changed headers to rebuilt TUs using Ninja's compiler dependency database."""
    commands = {normal(source_path(entry)): entry for entry in entries}
    changed_paths = {normal(root / path) for path in changed}
    units = set()
    covered = set()
    block = []

    def consume():
        deps = {normal(path) for path in block}
        matches = deps & changed_paths
        matching_units = deps & commands.keys()
        if matches and matching_units:
            units.update(matching_units)
            covered.update(matches)

    for line in dependencies.splitlines():
        if line and not line[0].isspace():
            consume()
            block = []
        elif line.strip():
            block.append(line.strip())
    consume()
    # A changed source itself is a valid TU, even when it has no header dependencies.
    for path in changed_paths & commands.keys():
        units.add(path)
        covered.add(path)
    missing = changed_paths - covered
    if missing:
        raise CheckError("No compilation/dependency coverage for: " + ", ".join(sorted(missing)))
    return sorted(units)


def check(changed, *, plan=False):
    cpp = [path for path in changed if own_cpp(path) and (ROOT / path).is_file()]
    native = any(build_input(path) for path in changed)
    python_checks = native or any(path.startswith(".githooks/") or
                                 (path.startswith("scripts/") and path.endswith(".py"))
                                 for path in changed)
    if plan:
        print(json.dumps({"format": cpp, "native_build_and_tests": native,
                          "python_tests": python_checks}, ensure_ascii=False, indent=2))
        return
    if not native and not python_checks:
        print("pre-commit: no C++ or Python changes to check.")
        return
    # Check the worktree/index agreement before running tests against worktree files.
    unstaged = git_paths("diff", "--name-only", "--ignore-submodules=all")
    untracked = git_paths("ls-files", "--others", "--exclude-standard")
    mismatch = [path for path in unstaged + untracked if build_input(path) or
                path.startswith(".githooks/") or
                (path.startswith("scripts/") and path.endswith(".py"))]
    if mismatch:
        raise CheckError("Stage or set aside these check inputs before checking: " + ", ".join(mismatch))
    if python_checks:
        execute([sys.executable, "-m", "unittest", "discover", "-s", "scripts/tests", "-v"])
    if not native:
        return

    formatter = executable(option("uncrustify", "uncrustify"))
    version = execute([formatter, "--version"], capture=True)
    if "Uncrustify-0.83.0" not in version:
        raise CheckError("Uncrustify 0.83.0 is required for the repository's style.")
    for path in cpp:
        execute([formatter, "--check", "-c", ROOT / "uncrustify.cfg", "-f", ROOT / path])

    cmake = executable("cmake")
    if os.name == "nt" and Path(cmake).is_absolute():
        # Git for Windows prepends its own MSYS DLLs to PATH in shell hooks.
        # Prefer the selected build toolchain's bin for GCC subprocess DLLs.
        os.environ["PATH"] = str(Path(cmake).parent) + os.pathsep + os.environ.get("PATH", "")
    ninja = executable("ninja")
    ctest = executable("ctest")
    tidy = executable(option("clangTidy", "clang-tidy"))
    powershell = executable("pwsh")
    build = Path(option("buildDirectory", "work/pre-commit-build"))
    if not build.is_absolute():
        build = ROOT / build
    jobs = int(option("jobs", "2"))
    if jobs < 1:
        raise CheckError("Hook jobs must be positive")
    execute([cmake, "-S", ROOT, "-B", build, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
             "-DCMAKE_POLICY_VERSION_MINIMUM=3.5", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
             "-DRB2D_BUILD_CORE_TESTS=ON", "-DRB2D_BUILD_EDITOR_TESTS=ON", "-DRB2D_EDITOR_GRAPHICS_TESTS=OFF"])
    execute([cmake, "--build", build, "--parallel", str(jobs), "--target",
             "robot2D", "robot2D-core-tests", "robot2D-editor-test-suite"])
    execute([ctest, "--test-dir", build, "-C", "Release", "--no-tests=error",
             "--output-on-failure", "--timeout", "60"])

    entries = json.loads((build / "compile_commands.json").read_text(encoding="utf-8"))
    entries = own_entries(entries)
    config_changed = any(build_input(path) and not own_cpp(path) for path in changed)
    if config_changed:
        units = [str(source_path(entry)) for entry in entries]
    elif cpp:
        deps = execute([ninja, "-C", build, "-t", "deps"], capture=True)
        units = affected_units(cpp, entries, deps, ROOT)
    else:
        units = []  # Deleted sources still triggered the build and test suite above.
    if units:
        files = build / "pre-commit-tidy-files.json"
        files.write_text(json.dumps(units), encoding="utf-8")
        execute([powershell, "-NoProfile", "-File", ROOT / "scripts/run_clang_tidy.ps1",
                 "-BuildDirectory", build, "-ClangTidy", tidy, "-FilesFrom", files, "-WarningsAsErrors"])
    print("pre-commit: checks passed; sources and index were not rewritten.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--plan", action="store_true", help="show selected checks without running tools")
    args = parser.parse_args()
    try:
        changed = git_paths("diff", "--cached", "--name-only", "--diff-filter=ACMRD")
        check(changed, plan=args.plan)
    except (CheckError, OSError, ValueError) as error:
        print(f"pre-commit: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
