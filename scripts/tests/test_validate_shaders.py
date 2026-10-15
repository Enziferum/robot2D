import contextlib
import importlib.util
import io
import os
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock


spec = importlib.util.spec_from_file_location("validate_shaders", Path(__file__).resolve().parents[1] / "validate_shaders.py")
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
GLSLANG = os.environ.get("GLSLANG_VALIDATOR") or shutil.which("glslang") or shutil.which("glslangValidator")


class ShaderValidationTests(unittest.TestCase):
    def test_extracts_actual_literal_and_maps_prefix_lines(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "shader.hpp"
            path.write_text('// header\nconst char* vertex = R"glsl(\nvoid main() {}\n)glsl";\n', encoding="utf-8")
            original, text, first = validator.prepare_source(root, {
                "path": "shader.hpp", "symbol": "vertex", "prefix": "#version 430 core\n"})
            self.assertEqual(text, "#version 430 core\n\nvoid main() {}\n")
            generated = root / "shader.vert"
            output = validator.remap_output(f"ERROR: {generated}:3: bad syntax", [(generated, original, first)])
            self.assertIn(f"{original}:3: bad syntax", output)
            with self.assertRaisesRegex(ValueError, "exactly one"):
                validator.prepare_source(root, {"path": "shader.hpp", "symbol": "missing"})

    def test_unregistered_shader_is_error(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "editor/res").mkdir(parents=True)
            (root / "editor/res/new.frag").write_text("void main() {}")
            with self.assertRaisesRegex(ValueError, "new.frag"):
                validator.check_coverage(root, [])

    def test_missing_validator_is_failure(self):
        with mock.patch.dict(os.environ, {}, clear=True), mock.patch.object(validator.shutil, "which", return_value=None), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(validator.main([]), 1)

    def test_missing_executable_is_failure(self):
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(validator.main(["--validator", "robot2d-no-such-validator"]), 1)

    @unittest.skipUnless(GLSLANG, "Khronos glslang is not installed")
    def test_real_syntax_and_link_errors(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            vertex = root / "test.vert"
            fragment = root / "test.frag"
            program = {"name": "fixture", "stages": [
                {"path": "test.vert", "stage": "vert"}, {"path": "test.frag", "stage": "frag"}]}
            vertex.write_text("#version 430 core\nout vec3 color; void main() { color = vec3(1); gl_Position = vec4(0); }")
            fragment.write_text("#version 430 core\nin vec3 color; out vec4 pixel; void main() { pixel = vec4(color, 1); }")
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertTrue(validator.validate_program(GLSLANG, root, program))
            fragment.write_text("#version 430 core\nin vec2 color; out vec4 pixel; void main() { pixel = vec4(color, 0, 1); }")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertFalse(validator.validate_program(GLSLANG, root, program))
            self.assertIn("link: FAILED", output.getvalue())
            fragment.write_text("#version 430 core\nvoid main() { broken syntax; }")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertFalse(validator.validate_program(GLSLANG, root, program))
            self.assertIn(str(fragment.resolve()) + ":2:", output.getvalue())

    @unittest.skipUnless(GLSLANG, "Khronos glslang is not installed")
    def test_wrong_tool_version_is_failure(self):
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(validator.main(["--require-version", "0.0.0"]), 1)


if __name__ == "__main__":
    unittest.main()
