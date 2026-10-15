"""Validate project GLSL stages and linked programs using Khronos glslang (no GPU)."""

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
RAW_STRING = re.compile(r'\b(\w+)\s*=\s*R"([^ ()\\\t\r\n]{0,16})\((.*?)\)\2"', re.DOTALL)


def prepare_source(root, spec):
    path = (root / spec["path"]).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError(f"Shader path escapes project: {path}")
    text = path.read_text(encoding="utf-8")
    line = 1
    if spec.get("symbol"):
        matches = [m for m in RAW_STRING.finditer(text) if m[1] == spec["symbol"]]
        if len(matches) != 1:
            raise ValueError(f"{path}: expected exactly one raw string for {spec['symbol']}")
        match = matches[0]
        line = text.count("\n", 0, match.start(3)) + 1
        text = match[3]
    prefix = spec.get("prefix", "")
    return path, prefix + text, line - prefix.count("\n")


def check_coverage(root, programs):
    registered = {(s["path"], s.get("symbol")) for p in programs for s in p["stages"]}
    discovered = set()
    for directory in ("core/include", "core/src", "editor/include", "editor/src", "editor/res"):
        for path in (root / directory).rglob("*"):
            if path.suffix in (".vert", ".frag", ".geom", ".glsl", ".comp", ".tesc", ".tese"):
                discovered.add((path.relative_to(root).as_posix(), None))
            elif path.suffix in (".hpp", ".cpp"):
                for match in RAW_STRING.finditer(path.read_text(encoding="utf-8")):
                    if re.search(r"\bvoid\s+main\s*\(", match[3]) and (
                            "#version" in match[3] or "gl_Position" in match[3] or "fragColor" in match[3]):
                        discovered.add((path.relative_to(root).as_posix(), match[1]))
    missing = discovered - registered
    if missing:
        raise ValueError("GLSL sources missing from scripts/shaders.json: " + ", ".join(
            f"{p}:{s or '<file>'}" for p, s in sorted(missing)))


def remap_output(output, mappings):
    if len(mappings) == 1:
        _, original, first_line = mappings[0]
        output = re.sub(r"(ERROR|WARNING): 0:(\d+):",
                        lambda m: f"{m[1]}: {original}:{max(1, first_line + int(m[2]) - 1)}:", output)
    for generated, original, first_line in mappings:
        # Glslang identifies stages by their input filename. Preserve non-location diagnostics too.
        pattern = re.escape(str(generated)) + r":(\d+):"
        output = re.sub(pattern, lambda m: f"{original}:{max(1, first_line + int(m[1]) - 1)}:", output)
        output = output.replace(str(generated), str(original))
    return output


def validate_program(validator, root, program):
    failed = False
    with tempfile.TemporaryDirectory(prefix="robot2d-glsl-") as temporary:
        stages = []
        mappings = []
        seen = set()
        for spec in program["stages"]:
            stage = spec["stage"]
            if stage not in ("vert", "frag", "geom", "comp", "tesc", "tese") or stage in seen:
                raise ValueError(f"{program['name']}: invalid or duplicate stage '{stage}'")
            seen.add(stage)
            original, text, first_line = prepare_source(root, spec)
            generated = Path(temporary) / f"shader.{stage}"
            generated.write_text(text, encoding="utf-8")
            stages.append(str(generated))
            mappings.append((generated, original, first_line))
        if not stages:
            raise ValueError(f"{program['name']}: program has no stages")
        # Plain GLSL validation: -G/-V would additionally impose SPIR-V requirements.
        for label, arguments in [(s, [s]) for s in stages] + [("link", ["-l", *stages])]:
            result = subprocess.run([validator, *arguments], capture_output=True, text=True,
                                    encoding="utf-8", errors="replace")
            raw = result.stdout + result.stderr
            raw = "\n".join(line for line in raw.splitlines() if line.strip() not in stages)
            active_mappings = [m for m in mappings if str(m[0]) == label] if label != "link" else mappings
            output = remap_output(raw, active_mappings).strip()
            if result.returncode or output:
                print(f"[{program['name']}] {Path(label).name}: {'FAILED' if result.returncode else 'diagnostics'}")
                if output:
                    print(output)
            failed |= result.returncode != 0
    print(f"{program['name']}: {'FAILED' if failed else 'passed'}")
    return not failed


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--validator", help="Path to glslang or glslangValidator; otherwise search PATH")
    parser.add_argument("--manifest", type=Path, default=ROOT / "scripts/shaders.json")
    parser.add_argument("--program", action="append", help="Select a program name; repeat to select several")
    parser.add_argument("--require-version", help="Require an exact glslang version (used by CI)")
    args = parser.parse_args(argv)
    try:
        validator = args.validator or os.environ.get("GLSLANG_VALIDATOR") or shutil.which("glslang") or shutil.which("glslangValidator")
        if not validator:
            raise ValueError("Khronos glslang not found; install it or pass --validator <executable>")
        programs = json.loads(args.manifest.read_text(encoding="utf-8"))["programs"]
        names = [p["name"] for p in programs]
        if not names or len(names) != len(set(names)):
            raise ValueError("Shader manifest must contain uniquely named programs")
        check_coverage(ROOT, programs)
        if args.program:
            unknown = set(args.program) - set(names)
            if unknown:
                raise ValueError(f"Unknown shader programs: {', '.join(sorted(unknown))}")
            programs = [p for p in programs if p["name"] in args.program]
        version = subprocess.run([validator, "--version"], check=True, capture_output=True, text=True)
        if args.require_version and not re.search(
                r"^Glslang Version: \d+:" + re.escape(args.require_version) + r"\s*$",
                version.stdout, re.MULTILINE):
            raise ValueError(f"Expected glslang {args.require_version}; got {version.stdout.strip()}")
        print(version.stdout.strip())
        results = [validate_program(validator, ROOT, program) for program in programs]
        return 0 if all(results) else 1
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f"GLSL validation failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
