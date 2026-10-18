"""Run the same Release build/tests on both CI platforms and retain diagnostics."""

import argparse
import html
import os
from pathlib import Path
import shlex
import subprocess
import sys
import xml.etree.ElementTree as ET


def test_report(xml_path):
    """Render CTest JUnit as escaped Markdown/HTML accepted by Actions Summary."""
    root = ET.parse(xml_path).getroot()
    cases = list(root.iter("testcase"))
    if not cases:
        raise ValueError("JUnit report contains no tests")
    counts = dict(passed=0, failed=0, skipped=0)
    rows = []
    details = []
    for case in cases:
        failure = case.find("failure")
        if failure is None:
            failure = case.find("error")
        skipped = case.find("skipped") is not None
        status = "failed" if failure is not None else "skipped" if skipped else "passed"
        counts[status] += 1
        name = html.escape(case.get("name", "unnamed")).replace("|", "&#124;").replace("\n", " ")
        rows.append(f"| {name} | {status} | {html.escape(case.get('time', ''))} |")
        if failure is not None or skipped:
            output = case.findtext("system-out", "")
            if failure is not None:
                output = f"{failure.get('message', '')}\n{failure.text or ''}\n{output}"
            # Full output is retained in XML/logs; keep Summary below GitHub's limit.
            details.append(f"<details><summary>{name}: {status}</summary>\n\n<pre>"
                           f"{html.escape(output[:6000])}</pre>\n</details>\n")
    text = (f"Tests: **{len(cases)}** — passed: **{counts['passed']}**, "
            f"failed: **{counts['failed']}**, skipped: **{counts['skipped']}**.\n\n"
            "| Test | Result | Seconds |\n|---|---|---|\n" + "\n".join(rows)
            + "\n\n" + "\n".join(details))
    return text, counts


def run(command, log_path, root):
    print(f"+ {shlex.join(command)}", flush=True)
    with log_path.open("w", encoding="utf-8") as log:
        log.write(shlex.join(command) + "\n")
        log.flush()
        with subprocess.Popen(command, cwd=root, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, text=True,
                              encoding="utf-8", errors="replace") as process:
            for line in process.stdout:
                print(line, end="", flush=True)
                log.write(line)
            return process.wait()


def main():
    # CMake/Mono diagnostics may contain non-ASCII text on Windows consoles.
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=Path("build"))
    parser.add_argument("--graphics", action="store_true")
    parser.add_argument("--parallel", type=int, default=2)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = (root / args.build_dir).resolve()
    reports = build / "reports"
    reports.mkdir(parents=True, exist_ok=True)
    junit = reports / "tests.xml"
    junit.unlink(missing_ok=True)  # Never publish stale results from a previous run.
    summary = "## robot2D — Release\n\n"
    result = 1
    phase = "configure"
    commands = [
        ("configure", ["cmake", "-S", str(root), "-B", str(build), "-G", "Ninja",
                       "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
                       "-DRB2D_BUILD_CORE_TESTS=ON", "-DRB2D_BUILD_EDITOR_TESTS=ON",
                       "-DRB2D_BUILD_CORE_EXAMPLES=ON",
                       f"-DRB2D_CORE_EXAMPLE_GRAPHICS_TESTS={'ON' if args.graphics else 'OFF'}",
                       f"-DRB2D_EDITOR_GRAPHICS_TESTS={'ON' if args.graphics else 'OFF'}"]),
        ("build", ["cmake", "--build", str(build), "--parallel", str(args.parallel),
                   "--target", "robot2D", "robot2D-core-tests", "robot2D-editor-test-suite",
                   "robot2D-first-sprite", "robot2D-keyboard-movement", "robot2D-keyboard-movement-test",
                   "robot2D-bounded-sprite", "robot2D-sprite-animation", "robot2D-camera-view", "robot2D-core-lessons-test",
                   "robot2D-sprite-collisions", "robot2D-sprite-collisions-test",
                   "robot2D-collect-items", "robot2D-collect-items-test",
                   "robot2D-game-states", "robot2D-game-states-test",
                   *(["robot2D-first-sprite-test", "robot2D-sprite-collisions-graphics-test",
                      "robot2D-collect-items-graphics-test", "robot2D-game-states-graphics-test"] if args.graphics else [])]),
        ("test", ["ctest", "--test-dir", str(build), "-C", "Release", "--no-tests=error",
                  "--output-on-failure", "--timeout", "60", "--output-junit", str(junit)]),
    ]
    try:
        for phase, command in commands:
            result = run(command, reports / f"{phase}.log", root)
            summary += f"- {phase}: {'passed' if result == 0 else f'FAILED (exit {result})'}\n"
            if result:
                break
        if junit.exists():
            text, counts = test_report(junit)
            summary += "\n" + text
            if counts["failed"] or (args.graphics and counts["skipped"]):
                result = result or 1
                summary += "\n\nFailures or unexpected skipped graphics tests require attention.\n"
        else:
            summary += "\nTests did not produce a JUnit report. See the stage logs.\n"
            result = result or 1
    except (OSError, ValueError, ET.ParseError) as error:
        summary += f"\n{phase}: {html.escape(str(error))}\n"
        print(error, file=sys.stderr)
        result = 1
    finally:
        summary += "\n\nDownload **test-report** under this run's **Artifacts** for JUnit XML, full logs and fixtures.\n"
        (reports / "summary.md").write_text(summary, encoding="utf-8")
        if os.environ.get("GITHUB_STEP_SUMMARY"):
            with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as output:
                output.write(summary)
    return result


if __name__ == "__main__":
    sys.exit(main())
