import argparse
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location('review', Path(__file__).resolve().parents[1] / 'review.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)


class ReviewTests(unittest.TestCase):
    def setUp(self):
        temporary = review.ROOT / 'work' / 'review-tests'
        temporary.mkdir(parents=True, exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(dir=temporary)
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.git('init', '-q')
        self.git('config', 'user.name', 'Review tests')
        self.git('config', 'user.email', 'review@example.invalid')
        (self.root / '.gitignore').write_text('work/\n', encoding='utf-8')
        (self.root / 'code.txt').write_text('original\n', encoding='utf-8')
        self.git('add', '.')
        self.git('-c', 'core.hooksPath=', 'commit', '-qm', 'Initial')
        self.initial = self.git('rev-parse', 'HEAD').decode().strip()

    def git(self, *args):
        return review.git(self.root, *args)

    def snapshot(self, **kwargs):
        args = argparse.Namespace(files=[], base=None, commit=None)
        for key, value in kwargs.items():
            setattr(args, key, value)
        return review.snapshot(self.root, args)

    def test_staged_unstaged_untracked_and_deleted(self):
        (self.root / 'code.txt').write_text('staged\n', encoding='utf-8')
        self.git('add', 'code.txt')
        (self.root / 'code.txt').write_text('unstaged\n', encoding='utf-8')
        (self.root / 'новый файл.txt').write_text('новый\n', encoding='utf-8')
        (self.root / '.gitignore').unlink()
        state, diff = self.snapshot()
        self.assertEqual({entry['path'] for entry in state['files']},
                         {'code.txt', '.gitignore', 'новый файл.txt'})
        self.assertIn(b'+unstaged', diff)
        self.assertNotIn(b'+staged', diff)
        self.assertTrue(next(entry for entry in state['files']
                             if entry['path'] == '.gitignore')['missing_or_submodule'])

    def test_literal_paths_and_directory_scope(self):
        (self.root / 'area').mkdir()
        (self.root / 'area' / '[test].txt').write_text('a', encoding='utf-8')
        (self.root / 'other.txt').write_text('b', encoding='utf-8')
        state, _ = self.snapshot(files=['area/[test].txt'])
        self.assertEqual([entry['path'] for entry in state['files']], ['area/[test].txt'])
        state, _ = self.snapshot(files=['area'])
        self.assertEqual(len(state['files']), 1)

    def test_rejects_external_paths(self):
        with self.assertRaises(ValueError):
            self.snapshot(files=['../outside'])
        with self.assertRaises(ValueError):
            self.snapshot(files=['.git/config'])

    def test_base_and_commit_exclude_working_changes(self):
        (self.root / 'code.txt').write_text('committed\n', encoding='utf-8')
        self.git('add', '.')
        self.git('-c', 'core.hooksPath=', 'commit', '-qm', 'Change')
        (self.root / 'code.txt').write_text('local\n', encoding='utf-8')
        for kwargs in ({'base': self.initial}, {'commit': 'HEAD'}):
            state, diff = self.snapshot(**kwargs)
            self.assertIn(b'+committed', diff)
            self.assertNotIn(b'+local', diff)
            self.assertEqual(state['baseline'], self.initial)

    def test_root_commit_and_bad_revision(self):
        state, diff = self.snapshot(commit=self.initial)
        self.assertIn(b'+original', diff)
        self.assertEqual(len(state['files']), 2)
        with self.assertRaises(RuntimeError):
            self.snapshot(base='nonexistent-ref')

    def invoke(self, args, runner=None):
        with patch.object(review, 'ROOT', self.root), \
                patch.object(review, 'run_model', side_effect=runner) as model, \
                patch('sys.stdout', new_callable=io.StringIO), \
                patch('sys.stderr', new_callable=io.StringIO):
            code = review.main(args)
        directories = list((self.root / 'work' / 'reviews').glob('*')) if (
            self.root / 'work' / 'reviews').exists() else []
        return code, model, directories

    def test_clean_tree_skips_model(self):
        code, model, directories = self.invoke([])
        self.assertEqual(code, 0)
        model.assert_not_called()
        self.assertFalse(directories)

    def test_prepare_copies_untracked_and_writes_chat_role(self):
        (self.root / 'new.txt').write_bytes(b'\x00\xffdata')
        code, model, directories = self.invoke(['--prepare-only'])
        self.assertEqual(code, 0)
        model.assert_not_called()
        directory = directories[0]
        manifest = json.loads((directory / 'manifest.json').read_text(encoding='utf-8'))
        entry = manifest['scope']['files'][0]
        self.assertEqual((directory / entry['snapshot']).read_bytes(), b'\x00\xffdata')
        self.assertEqual(manifest['status'], 'prepared')
        self.assertIn('[Review]', (directory / 'chat-prompt.txt').read_text(encoding='utf-8'))
        self.assertFalse((directory / 'report.md').exists())

    def test_model_success_and_concurrent_change(self):
        (self.root / 'code.txt').write_text('before', encoding='utf-8')

        def runner(command, root, prompt, directory):
            self.assertEqual(command[command.index('--sandbox') + 1], 'read-only')
            (directory / 'report.md').write_text('Замечаний нет', encoding='utf-8')
            (root / 'code.txt').write_text('during', encoding='utf-8')
            return 0, 'session-for-test'

        code, _, directories = self.invoke([], runner)
        self.assertEqual(code, 0)
        manifest = json.loads((directories[0] / 'manifest.json').read_text(encoding='utf-8'))
        self.assertTrue(manifest['workspace_changed'])
        self.assertEqual(manifest['session_id'], 'session-for-test')

    def test_model_failure_and_missing_executable_are_not_success(self):
        (self.root / 'code.txt').write_text('changed', encoding='utf-8')
        for runner in (lambda *args: (0, None), lambda *args: (7, None),
                       FileNotFoundError('missing codex')):
            code, _, directories = self.invoke([], runner)
            self.assertNotEqual(code, 0)
            self.assertIn('failed', [(json.loads((item / 'manifest.json').read_text(
                encoding='utf-8')))['status'] for item in directories])

    def test_checks_require_existing_build(self):
        with patch.object(review, 'ROOT', self.root), patch('sys.stderr', new_callable=io.StringIO):
            with self.assertRaises(SystemExit):
                review.main(['--run-checks'])
            with self.assertRaises(SystemExit):
                review.main(['--run-checks', '--build-directory', 'missing'])

    def test_cli_event_stream_and_error_with_zero_exit(self):
        fake = self.root / 'fake_codex.py'
        fake.write_text('''import json
import sys
sys.stdin.read()
print(json.dumps({'type': 'thread.started', 'thread_id': 'fake-session'}))
print(json.dumps({'type': 'item.completed', 'item': {'type': 'agent_message', 'text': 'review'}}))
if '--fail' in sys.argv:
    print(json.dumps({'type': 'turn.failed', 'error': {'message': 'failed'}}))
print('diagnostic', file=sys.stderr)
''', encoding='utf-8')
        for fail in (False, True):
            directory = self.root / ('failure' if fail else 'success')
            directory.mkdir()
            with patch('sys.stdout', new_callable=io.StringIO):
                code, session = review.run_model(
                    [sys.executable, str(fake)] + (['--fail'] if fail else []),
                    self.root, 'review prompt', directory)
            self.assertEqual(code, 1 if fail else 0)
            self.assertEqual(session, 'fake-session')
            self.assertIn('diagnostic', (directory / 'stderr.log').read_text(encoding='utf-8'))
            self.assertTrue((directory / 'events.jsonl').stat().st_size)


if __name__ == '__main__':
    unittest.main()
