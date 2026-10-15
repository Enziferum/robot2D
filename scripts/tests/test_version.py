"""Exercise the real CMake version generator without engine dependencies."""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
CMAKE = shutil.which("cmake")
GIT = shutil.which("git")


class VersionTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(CMAKE, "Version fixtures require CMake in PATH")
        self.assertIsNotNone(GIT, "Version fixtures require Git in PATH")
        self.assertIsNotNone(shutil.which("ninja"), "Version fixtures require Ninja in PATH")
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.source = self.root / "source"
        self.source.mkdir()
        # A commit hook's GIT_INDEX_FILE/GIT_DIR must not leak into fixture repos.
        self.env = {key: value for key, value in os.environ.items() if not key.startswith("GIT_")}
        self.write_project(self.source)

    def write_project(self, source):
        (source / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.21)\n'
            'project(VersionProbe VERSION 2.3.4 LANGUAGES NONE)\n'
            f'include("{(ROOT / "cmake/Version.cmake").as_posix()}")\n', encoding="utf-8")

    def run_tool(self, command, cwd=None, success=True):
        result = subprocess.run(command, cwd=cwd or self.source, env=self.env,
                                capture_output=True, text=True, encoding="utf-8", errors="replace")
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def configure(self, *options, success=True):
        return self.run_tool([CMAKE, "-S", str(self.source), "-B", str(self.root / "build"),
                              "-G", "Ninja", *options], success=success)

    def definitions(self):
        header = (self.root / "build/generated/include/robot2D/Version.hpp").read_text(encoding="utf-8")
        return dict(re.findall(r'^#define (RB2_\w+) "?([^"\n]+)"?$', header, re.MULTILINE))

    def git(self, *args, cwd=None):
        return self.run_tool([GIT, *args], cwd=cwd).stdout.strip()

    def commit(self, cwd=None):
        # Fixture repositories do not use the project's installed commit hooks.
        self.git("-c", "user.name=Version Tests", "-c", "user.email=version@example.invalid",
                 "-c", "core.hooksPath=" + str(self.root / "no-hooks"),
                 "commit", "--allow-empty", "-m", "fixture", cwd=cwd)
        return self.git("rev-parse", "HEAD", cwd=cwd)

    def test_archive_and_stable_release_have_deterministic_versions(self):
        self.configure("-DRB2D_EDITOR_BUILD_ID=package.123")
        definitions = self.definitions()
        self.assertEqual(definitions["RB2_VERSION_FULL"], "2.3.4-dev+local")
        self.assertEqual(definitions["RB2_GIT_SHA"], "unknown")
        self.assertEqual(definitions["RB2_BUILD_ID"], "package.123")
        self.configure("-DRB2D_VERSION_PRERELEASE=")
        self.assertEqual(self.definitions()["RB2_VERSION_FULL"], "2.3.4")

    def test_git_sha_refreshes_after_commit_without_manual_configure(self):
        self.git("init")
        first = self.commit()
        self.configure()
        self.assertEqual(self.definitions()["RB2_VERSION_FULL"], "2.3.4-dev+g" + first[:12])
        # Ensure HEAD actually advances even if both commits run in the same second.
        self.git("-c", "user.name=Version Tests", "-c", "user.email=version@example.invalid",
                 "-c", "core.hooksPath=" + str(self.root / "no-hooks"),
                 "commit", "--allow-empty", "-m", "second fixture")
        second = self.git("rev-parse", "HEAD")
        self.assertNotEqual(first, second)
        self.run_tool([CMAKE, "--build", str(self.root / "build")])
        self.assertEqual(self.definitions()["RB2_GIT_SHA"], second)

    def test_archive_nested_in_another_repo_does_not_borrow_its_sha(self):
        self.git("init", cwd=self.root)
        self.commit(cwd=self.root)
        self.configure()
        self.assertEqual(self.definitions()["RB2_GIT_SHA"], "unknown")

    def test_linked_worktree_and_detached_head_report_their_own_sha(self):
        self.git("init")
        revision = self.commit()
        linked = self.root / "linked"
        self.git("worktree", "add", "-b", "linked-version", str(linked))
        self.source = linked
        self.write_project(linked)
        self.git("checkout", "--detach", revision)
        self.configure()
        self.assertEqual(self.definitions()["RB2_GIT_SHA"], revision)

    def test_prerelease_and_build_id_are_validated(self):
        self.configure("-DRB2D_VERSION_PRERELEASE=rc.1")
        self.assertEqual(self.definitions()["RB2_VERSION"], "2.3.4-rc.1")
        for value in ("rc..1", "rc.01", 'dev"bad', "dev;bad"):
            self.configure("-DRB2D_VERSION_PRERELEASE=" + value, success=False)
        self.configure("-DRB2D_VERSION_PRERELEASE=dev", '-DRB2D_EDITOR_BUILD_ID=bad"id', success=False)


if __name__ == "__main__":
    unittest.main()
