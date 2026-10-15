"""Require a registered task ID before the commit description."""

import argparse
import json
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
SUBJECT = re.compile(r"([A-Z][A-Z0-9]*-[0-9]{2,}):[ \t]+(\S.*)")


def validate(message, tasks):
    lines = message.splitlines()
    match = SUBJECT.fullmatch(lines[0]) if lines else None
    if not match:
        return "Start the first line with TASK-ID: description (example: LOAD-01: handle load failure)."
    if match.group(1) not in tasks:
        return f"Unknown task {match.group(1)}. Register it in .github/tasks.json before committing."
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("message_file", type=Path)
    args = parser.parse_args()
    try:
        tasks = json.loads((ROOT / ".github/tasks.json").read_text(encoding="utf-8"))["tasks"]
        error = validate(args.message_file.read_text(encoding="utf-8-sig"), tasks)
        if error:
            print(f"commit-msg: {error}", file=sys.stderr)
            return 1
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"commit-msg: validation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
