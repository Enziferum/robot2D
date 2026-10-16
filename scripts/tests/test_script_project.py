"""Real compiler tests for generated user projects and failed rebuilds."""
from pathlib import Path
import importlib.util
import json
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "editor/res/gen/gen_project.py"
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("user_script_project", SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
MONO = shutil.which("mono") or "C:/Program Files/Mono/bin/mono.exe"
MCS = shutil.which("mcs") or "C:/Program Files/Mono/bin/mcs.bat"


@unittest.skipUnless(Path(MONO).is_file() and Path(MCS).is_file(), "requires real Mono compiler")
class UserScriptProjectTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="robot2D user scripts ")
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name) / "project & spaces"
        self.root.mkdir()
        self.core = Path(self.directory.name) / "robot2D_ScriptCore.dll"
        source = Path(self.directory.name) / "Core.cs"
        source.write_text("namespace robot2D { public class Entity {} }", encoding="utf-8")
        subprocess.run([*module.compiler_command(MCS, MONO), "-target:library", "-out:" + str(self.core), str(source)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        self.name = "Arena & example"
        self.sources = self.root / "author sources"

    def build(self):
        module.build(self.root, self.name, self.core, Path(MCS), Path(MONO))
        return self.root / "assets/scripts/bin" / (self.name + ".dll")

    def test_generate_preserves_author_files_and_compiles_new_nested_sources(self):
        module.generate(self.root, self.name, self.sources, self.core)
        project = self.sources / (self.name + ".csproj")
        original = project.read_bytes()
        user = self.sources / "nested/Enemy.cs"
        user.parent.mkdir()
        user.write_text("public class Enemy : robot2D.Entity {}", encoding="utf-8")
        module.generate(self.root, self.name, self.sources, self.core)
        self.assertEqual(project.read_bytes(), original)
        self.assertTrue(self.build().is_file())
        config = json.loads((self.root / module.CONFIG).read_text(encoding="utf-8"))
        self.assertEqual(config["sources"], "author sources")

    def test_failed_build_invalidates_dll_and_recovers(self):
        module.generate(self.root, self.name, self.sources, self.core)
        dll = self.build()
        (self.sources / "Error.cs").write_text("not valid C#", encoding="utf-8")
        with self.assertRaises(ValueError):
            self.build()
        self.assertFalse(dll.exists())
        self.assertFalse((self.root / ".robot2d/scripts-build.json").exists())
        (self.sources / "Error.cs").unlink()
        self.assertTrue(self.build().is_file())

    def test_missing_compiler_and_reference_fail_explicitly(self):
        module.generate(self.root, self.name, self.sources, self.core)
        dll = self.build()
        with self.assertRaises(OSError):
            module.build(self.root, self.name, self.core, self.root / "missing-mcs", Path(MONO))
        self.assertFalse(dll.exists())
        self.build()
        self.core.unlink()
        with self.assertRaisesRegex(ValueError, "ScriptCore is missing"):
            self.build()
        self.assertFalse(dll.exists())

    def test_compile_only_legacy_project_is_not_overwritten(self):
        dll = self.root / "assets/scripts/bin" / (self.name + ".dll")
        dll.parent.mkdir(parents=True)
        dll.write_bytes(b"original legacy DLL")
        with self.assertRaisesRegex(ValueError, "only a compiled DLL"):
            self.build()
        self.assertEqual(dll.read_bytes(), b"original legacy DLL")

    def test_reference_change_rebuilds_against_new_api(self):
        module.generate(self.root, self.name, self.sources, self.core)
        self.build()
        source = self.core.with_suffix(".cs")
        source.write_text("namespace robot2D { public class DifferentApi {} }", encoding="utf-8")
        subprocess.run([*module.compiler_command(MCS, MONO), "-target:library", "-out:" + str(self.core), str(source)], check=True)
        with self.assertRaises(ValueError):
            self.build()


class InvalidScriptProjectTests(unittest.TestCase):
    def test_unsafe_names_are_rejected(self):
        for name in ["", "..", "a/b", "a\\b", "x\x00y", "x\ny"]:
            with self.subTest(name=repr(name)), self.assertRaises(ValueError):
                module.project_name(name)


if __name__ == "__main__":
    unittest.main()
