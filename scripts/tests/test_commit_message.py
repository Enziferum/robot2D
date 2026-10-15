import importlib.util
from pathlib import Path
import unittest


spec = importlib.util.spec_from_file_location("commit_message",
                                             Path(__file__).resolve().parents[1] / "check_commit_message.py")
commit_message = importlib.util.module_from_spec(spec)
spec.loader.exec_module(commit_message)


class CommitMessageTests(unittest.TestCase):
    tasks = {"LOAD-01": "Load", "PROCESS-01": "Workflow"}

    def test_task_precedes_description(self):
        for text in ("LOAD-01: Fix loading", "PROCESS-01: Правила коммитов\n\nПодробности."):
            with self.subTest(text=text):
                self.assertIsNone(commit_message.validate(text, self.tasks))

    def test_rejects_missing_misplaced_or_empty_description(self):
        for text in ("", "\nLOAD-01: description", "Fix loading LOAD-01", "LOAD-01", "LOAD-01:",
                     "LOAD-01:   ", "load-01: Fix", " LOAD-01: Fix", "LOAD-01 Fix", "LOAD-1: Fix"):
            with self.subTest(text=text):
                self.assertIsNotNone(commit_message.validate(text, self.tasks))

    def test_rejects_unregistered_task(self):
        self.assertIn("Unknown task", commit_message.validate("LOAD-02: Fix", self.tasks))

    def test_automatic_messages_have_no_exception(self):
        for text in ("Merge branch 'dev'", 'Revert "Fix"', "fixup! LOAD-01: Fix"):
            with self.subTest(text=text):
                self.assertIsNotNone(commit_message.validate(text, self.tasks))
