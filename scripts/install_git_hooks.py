"""Enable the repository's versioned hooks for this checkout."""

from pathlib import Path
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parents[1]
    current = subprocess.run(["git", "config", "--get", "core.hooksPath"], cwd=root,
                             capture_output=True, text=True)
    if current.returncode not in (0, 1):
        print(current.stderr, file=sys.stderr)
        return current.returncode
    if current.stdout.strip() not in ("", ".githooks"):
        print(f"Existing core.hooksPath={current.stdout.strip()} was preserved. "
              "Integrate the hooks manually; see docs/project-workflow.md.", file=sys.stderr)
        return 1
    # POSIX needs the executable bit; Windows Git recognizes the shell shebang.
    hook = root / ".githooks/commit-msg"
    hook.chmod(hook.stat().st_mode | 0o111)
    subprocess.run(["git", "config", "--local", "core.hooksPath", ".githooks"],
                   cwd=root, check=True)
    print("Enabled .githooks: commit subjects must start with a registered TASK-ID: description.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
