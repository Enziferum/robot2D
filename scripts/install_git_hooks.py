"""Enable the repository's versioned hooks for this checkout."""

from pathlib import Path
import argparse
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--uncrustify", help="Uncrustify 0.83.0 executable, if absent from PATH")
    parser.add_argument("--clang-tidy", help="clang-tidy executable, if absent from PATH")
    parser.add_argument("--build-dir", help="Dedicated Ninja/Release hook build directory")
    parser.add_argument("--jobs", type=int, help="Parallel build jobs")
    args = parser.parse_args()
    if args.jobs is not None and args.jobs < 1:
        parser.error("--jobs must be positive")
    root = Path(__file__).resolve().parents[1]
    current = subprocess.run(["git", "config", "--get", "core.hooksPath"], cwd=root,
                             capture_output=True, text=True)
    if current.returncode not in (0, 1):
        print(current.stderr, file=sys.stderr)
        return current.returncode
    if current.stdout.strip() not in ("", ".githooks"):
        print(f"Existing core.hooksPath={current.stdout.strip()} was preserved. "
              "Integrate the hooks manually; see AGENTS.md and .githooks/.", file=sys.stderr)
        return 1
    # POSIX needs the executable bit; Windows Git recognizes the shell shebang.
    for name in ("commit-msg", "pre-commit"):
        hook = root / ".githooks" / name
        hook.chmod(hook.stat().st_mode | 0o111)
    for name, value in (("uncrustify", args.uncrustify), ("clangTidy", args.clang_tidy),
                        ("buildDirectory", args.build_dir), ("jobs", args.jobs)):
        if value is not None:
            subprocess.run(["git", "config", "--local", f"robot2d.hooks.{name}", str(value)],
                           cwd=root, check=True)
    subprocess.run(["git", "config", "--local", "core.hooksPath", ".githooks"],
                   cwd=root, check=True)
    print("Enabled .githooks: pre-commit checks and TASK-ID: description in commit-msg.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
