import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock
import xml.etree.ElementTree as ET


spec = importlib.util.spec_from_file_location("ci", Path(__file__).resolve().parents[1] / "ci.py")
ci = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ci)


class ReportTests(unittest.TestCase):
    def report(self, xml):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "tests.xml"
            path.write_text(xml, encoding="utf-8")
            return ci.test_report(path)

    def test_results_and_escaped_diagnostics(self):
        text, counts = self.report('''<testsuites><testsuite>
            <testcase name="ok" time="0.1"/>
            <testcase name="bad|name"><failure message="broken">&lt;script&gt;</failure>
              <system-out>log &amp; reason</system-out></testcase>
            <testcase name="no-context"><skipped/></testcase>
            <testcase name="crash"><error message="exception"/></testcase>
            </testsuite></testsuites>''')
        self.assertEqual(counts, dict(passed=1, failed=2, skipped=1))
        self.assertIn("bad&#124;name", text)
        self.assertIn("&lt;script&gt;", text)
        self.assertIn("log &amp; reason", text)
        self.assertNotIn("<script>", text)

    def test_empty_report_is_not_success(self):
        with self.assertRaises(ValueError):
            self.report("<testsuite tests='0'/>")

    def test_malformed_report_is_not_success(self):
        with self.assertRaises(ET.ParseError):
            self.report("<testsuite>")

    def test_limits_summary_output(self):
        text, _ = self.report('<testsuite><testcase name="bad"><failure>' + 'x' * 20000
                              + '</failure></testcase></testsuite>')
        self.assertLess(len(text), 7000)

    def test_build_failure_keeps_failure_status_and_summary(self):
        with tempfile.TemporaryDirectory() as directory:
            summary = Path(directory) / "github-summary.md"
            with mock.patch.object(ci.sys, "argv", ["ci.py", "--build-dir", directory]), \
                    mock.patch.dict(ci.os.environ, {"GITHUB_STEP_SUMMARY": str(summary)}), \
                    mock.patch.object(ci, "run", side_effect=[0, 7]) as run:
                self.assertEqual(ci.main(), 7)
            self.assertEqual(run.call_count, 2)
            self.assertIn("build: FAILED (exit 7)", summary.read_text(encoding="utf-8"))
            self.assertTrue((Path(directory) / "reports" / "summary.md").is_file())
