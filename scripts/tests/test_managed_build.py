"""Real Mono/CMake regressions for paths, dependencies and failed C# rebuilds."""

from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / "editor/cmake/MonoAssembly.cmake"


def mono_tool(name):
    found = shutil.which(name)
    if found:
        return found
    suffix = ".bat" if name == "mcs" else ".exe"
    candidate = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Mono/bin" / (name + suffix)
    return str(candidate) if candidate.is_file() else None


@unittest.skipUnless(shutil.which("cmake") and shutil.which("ninja") and mono_tool("mcs") and mono_tool("mono"),
                     "requires CMake, Ninja and Mono compiler/runtime")
class ManagedBuildTests(unittest.TestCase):
    def run_command(self, command, succeeds=True):
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=90)
        if succeeds:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def test_incremental_reference_changes_failure_and_recovery_with_spaces(self):
        with tempfile.TemporaryDirectory(prefix="robot2D managed ") as directory:
            root = Path(directory)
            source = root / "source tree"
            build = root / "build tree"
            output = build / "assembly output"
            source.mkdir()
            api = source / "Api.cs"
            api.write_text("public static class Api { public static int Value() { return 1; } }", encoding="utf-8")
            (source / "Consumer.cs").write_text(
                "class Consumer { static int Main() { return Api.Value() == 1 ? 0 : 1; } }", encoding="utf-8")
            compiler = Path(mono_tool("mcs")).as_posix()
            (source / "CMakeLists.txt").write_text(f"""cmake_minimum_required(VERSION 3.21)
project(ManagedFixture LANGUAGES NONE)
set(MONO_MCS_EXECUTABLE "{compiler}")
include("{MODULE.as_posix()}")
add_mono_assembly(TARGET Api SOURCES Api.cs DESTINATION "${{CMAKE_BINARY_DIR}}/assembly output")
add_mono_assembly(TARGET Consumer TYPE exe SOURCES Consumer.cs DESTINATION "${{CMAKE_BINARY_DIR}}/assembly output"
    REFERENCES "${{Api_OUTPUT}}")
add_dependencies(Consumer Api)
""", encoding="utf-8")
            self.run_command(["cmake", "-S", str(source), "-B", str(build), "-G", "Ninja"])
            command = ["cmake", "--build", str(build), "--target", "Consumer"]
            self.run_command(command)
            dll = output / "Api.dll"
            exe = output / "Consumer.exe"
            original = (dll.stat().st_mtime_ns, exe.stat().st_mtime_ns)
            self.run_command([mono_tool("mono"), str(exe)])
            self.run_command(command)
            self.assertEqual(original, (dll.stat().st_mtime_ns, exe.stat().st_mtime_ns))
            time.sleep(1.05)
            api.write_text("public static class Api { public static int Value() { return 2; } }", encoding="utf-8")
            self.run_command(command)
            self.assertNotEqual(original[0], dll.stat().st_mtime_ns)
            self.assertNotEqual(original[1], exe.stat().st_mtime_ns)
            self.run_command([mono_tool("mono"), str(exe)], succeeds=False)
            time.sleep(1.05)
            api.write_text("this is not C#", encoding="utf-8")
            diagnostic = self.run_command(command, succeeds=False)
            self.assertIn("Api.cs", diagnostic)
            self.assertFalse(dll.exists(), "failed build must not leave an old API DLL")
            api.write_text("public static class Api { public static int Value() { return 1; } }", encoding="utf-8")
            self.run_command(command)
            self.run_command([mono_tool("mono"), str(exe)])
            dll.unlink()
            self.run_command(command)
            self.assertTrue(dll.exists())

    def test_missing_compiler_reports_actionable_configuration_error(self):
        with tempfile.TemporaryDirectory(prefix="robot2D compiler ") as directory:
            root = Path(directory)
            (root / "Api.cs").write_text("class Api {}", encoding="utf-8")
            (root / "CMakeLists.txt").write_text(f"""cmake_minimum_required(VERSION 3.21)
project(MissingCompiler LANGUAGES NONE)
set(MONO_MCS_EXECUTABLE "${{CMAKE_SOURCE_DIR}}/missing mcs")
include("{MODULE.as_posix()}")
add_mono_assembly(TARGET Api SOURCES Api.cs)
""", encoding="utf-8")
            diagnostic = self.run_command(["cmake", "-S", str(root), "-B", str(root / "build"), "-G", "Ninja"], succeeds=False)
            self.assertIn("C# compiler not found", diagnostic)
            self.assertIn("MONO_MCS_EXECUTABLE", diagnostic)


if __name__ == "__main__":
    unittest.main()
