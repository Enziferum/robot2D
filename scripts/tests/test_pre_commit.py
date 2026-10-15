"""Regression checks for scope, dependency coverage and fail-closed hooks."""

import importlib.util
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location(
    "pre_commit", Path(__file__).resolve().parents[1] / "pre_commit.py")
hook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook)


class PreCommitTests(unittest.TestCase):
    def test_generated_version_templates_trigger_native_checks(self):
        for path in ("core/include/robot2D/Version.hpp.in", "editor/cmake/BuildInfo.hpp.in"):
            with self.subTest(path=path), patch("sys.stdout", new_callable=io.StringIO) as output:
                hook.check([path], plan=True)
                plan = json.loads(output.getvalue())
                self.assertTrue(plan["native_build_and_tests"])
                self.assertEqual(plan["format"], [])

    def test_documentation_and_dependencies_do_not_invoke_native_tools(self):
        for paths in (["docs/editor-roadmap.md"], ["editor/extlibs/yaml-cpp/src/emit.cpp"],
                      ["core/ext/imgui/imgui.cpp"], ["editor/src-backup/foo.cpp"],
                      ["core/src/Graphics/internal/stb_image_aug.c"],
                      ["core/include/robot2D/Graphics/internal/glad.h"]):
            with self.subTest(paths=paths), patch.object(hook, "execute") as run:
                hook.check(paths)
                run.assert_not_called()

    def test_python_changes_run_regressions_without_native_tools(self):
        with patch.object(hook, "execute") as run, patch.object(hook, "git_paths", return_value=[]), \
                patch.object(hook, "executable") as tool:
            hook.check(["scripts/pre_commit.py"])
            self.assertIn("unittest", run.call_args.args[0])
            tool.assert_not_called()

    def test_partial_staging_blocks_before_formatter_build_or_tests(self):
        with patch.object(hook, "execute"), patch.object(hook, "git_paths", side_effect=[
                ["editor/include/editor/SelectionService.hpp"], []]), \
                patch.object(hook, "executable") as tool:
            with self.assertRaisesRegex(hook.CheckError, "Stage or set aside"):
                hook.check(["editor/src/SelectionService.cpp"])
            tool.assert_not_called()

    def test_untracked_native_input_also_blocks(self):
        with patch.object(hook, "execute"), patch.object(hook, "git_paths", side_effect=[
                [], ["editor/src/NotStaged.cpp"]]):
            with self.assertRaisesRegex(hook.CheckError, "NotStaged.cpp"):
                hook.check(["CMakeLists.txt"])

    def test_command_failure_is_never_a_success(self):
        for code in (1, 77):
            with patch.object(hook.subprocess, "run", return_value=subprocess.CompletedProcess(
                    ["tool"], code, stdout="", stderr="diagnostic")):
                with self.assertRaises(hook.CheckError):
                    hook.execute(["tool"], capture=True)

    def test_dependency_mapping_only_selects_affected_own_units(self):
        root = hook.ROOT
        a, b = root / "editor/src/A.cpp", root / "editor/src/B.cpp"
        header = root / "editor/include/editor/A.hpp"
        entries = [{"file": str(path), "directory": str(root)} for path in (a, b)]
        dependencies = f"a.o: #deps 2\n    {a}\n    {header}\n\nb.o: #deps 1\n    {b}\n"
        self.assertEqual(hook.affected_units(["editor/include/editor/A.hpp"], entries,
                                           dependencies, root), [hook.normal(a)])

    def test_header_without_own_compilation_coverage_blocks(self):
        header = hook.ROOT / "editor/include/editor/Unused.hpp"
        deps = f"external.o: #deps 1\n    {header}\n"
        with self.assertRaisesRegex(hook.CheckError, "No compilation/dependency coverage"):
            hook.affected_units(["editor/include/editor/Unused.hpp"], [], deps, hook.ROOT)

    def test_compile_database_excludes_extlibs_and_outside_sources(self):
        paths = [hook.ROOT / "editor/src/main.cpp", hook.ROOT / "editor/extlibs/lib/src.cpp",
                 hook.ROOT.parent / "other.cpp"]
        entries = [{"directory": str(hook.ROOT), "file": str(path)} for path in paths]
        self.assertEqual(hook.own_entries(entries), entries[:1])

    def test_formatter_failure_stops_before_build_and_does_not_rewrite(self):
        commands = []

        def run(command, **kwargs):
            commands.append([str(value) for value in command])
            if "--version" in command:
                return "Uncrustify-0.83.0_f"
            if "--check" in command:
                raise hook.CheckError("format mismatch")
            return ""

        with patch.object(hook, "execute", side_effect=run), \
                patch.object(hook, "git_paths", return_value=[]), \
                patch.object(hook, "option", side_effect=lambda name, default: default), \
                patch.object(hook, "executable", side_effect=lambda name: name):
            with self.assertRaisesRegex(hook.CheckError, "format mismatch"):
                hook.check(["editor/src/SelectionService.cpp"])
        self.assertEqual(commands[-1][1], "--check")
        self.assertFalse(any("--replace" in cmd or "--build" in cmd or "add" in cmd
                             for cmd in commands))

    def test_native_pipeline_propagates_build_test_and_lint_failures(self):
        for failure in (None, "build", "ctest", "tidy"):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as temp:
                root = Path(temp).resolve()
                source = root / "editor/src/Probe.cpp"
                source.parent.mkdir(parents=True)
                original = "int probe() { return 0; }\n"
                source.write_text(original)
                build = root / "build"
                build.mkdir()
                (build / "compile_commands.json").write_text(json.dumps([
                    {"file": str(source), "directory": str(root)}]))
                commands = []

                def run(command, **kwargs):
                    command = [str(value) for value in command]
                    commands.append(command)
                    if "--version" in command:
                        return "Uncrustify-0.83.0_f"
                    if "-t" in command:
                        return f"probe.o: #deps 1\n    {source}\n"
                    stage = ("build" if "--build" in command else
                             "ctest" if command[0] == "ctest" else
                             "tidy" if "-FilesFrom" in command else None)
                    if stage == failure and failure is not None:
                        raise hook.CheckError(f"{stage} failed")
                    return ""

                with patch.object(hook, "ROOT", root), \
                        patch.object(hook, "execute", side_effect=run), \
                        patch.object(hook, "git_paths", return_value=[]), \
                        patch.object(hook, "option", side_effect=lambda name, default:
                                     "build" if name == "buildDirectory" else default), \
                        patch.object(hook, "executable", side_effect=lambda value: value):
                    if failure:
                        with self.assertRaisesRegex(hook.CheckError, f"{failure} failed"):
                            hook.check(["editor/src/Probe.cpp"])
                    else:
                        hook.check(["editor/src/Probe.cpp"])
                        self.assertIn("-WarningsAsErrors", commands[-1])
                        selected = json.loads((build / "pre-commit-tidy-files.json").read_text())
                        self.assertEqual(selected, [hook.normal(source)])
                self.assertEqual(source.read_text(), original)
                if failure == "build":
                    self.assertFalse(any(command[0] == "ctest" for command in commands))
                if failure == "ctest":
                    self.assertFalse(any("-FilesFrom" in command for command in commands))


if __name__ == "__main__":
    unittest.main()
