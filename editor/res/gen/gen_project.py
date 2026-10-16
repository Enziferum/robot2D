"""Generate and build robot2D user scripts without a shell or mutable templates."""
from pathlib import Path
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import uuid
import xml.etree.ElementTree as ET

CONFIG = ".robot2d/scripts.json"


def project_name(value):
    if not value or value in (".", "..") or any(c in value for c in '/\\\x00\r\n:<>"|?*'):
        raise ValueError("invalid project name")
    return value


def atomic_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding="utf-8")
    temporary.replace(path)


def generate(root, name, sources, core):
    name = project_name(name)
    root, sources, core = root.resolve(), sources.resolve(), core.resolve()
    if not core.is_file():
        raise ValueError("ScriptCore is missing: " + str(core))
    sources.mkdir(parents=True, exist_ok=True)
    project = sources / (name + ".csproj")
    # Never replace an author's existing IDE project.
    if not project.exists():
        ns = "http://schemas.microsoft.com/developer/msbuild/2003"
        ET.register_namespace("", ns)
        tag = lambda text: "{" + ns + "}" + text
        tree = ET.Element(tag("Project"), ToolsVersion="4.0", DefaultTargets="Build")
        props = ET.SubElement(tree, tag("PropertyGroup"))
        for key, value in {"ProjectGuid": "{" + str(uuid.uuid4()).upper() + "}",
                           "OutputType": "Library", "AssemblyName": name,
                           "RootNamespace": re.sub(r"[^a-zA-Z0-9_]", "_", "Game_" + name),
                           "TargetFrameworkVersion": "v4.8", "LangVersion": "7.2",
                           "OutputPath": "bin/$(Configuration)/", "WarningLevel": "4"}.items():
            ET.SubElement(props, tag(key)).text = value
        refs = ET.SubElement(tree, tag("ItemGroup"))
        ET.SubElement(refs, tag("Reference"), Include="System")
        reference = ET.SubElement(refs, tag("Reference"), Include="robot2D_ScriptCore")
        ET.SubElement(reference, tag("HintPath")).text = "References/robot2D_ScriptCore.dll"
        items = ET.SubElement(tree, tag("ItemGroup"))
        ET.SubElement(items, tag("Compile"), Include="**/*.cs", Exclude="bin/**;obj/**;References/**")
        ET.SubElement(tree, tag("Import"), Project="$(MSBuildToolsPath)/Microsoft.CSharp.targets")
        ET.ElementTree(tree).write(project, encoding="utf-8", xml_declaration=True)
    refs = sources / "References"
    refs.mkdir(exist_ok=True)
    if core != (refs / core.name).resolve():
        shutil.copy2(core, refs / core.name)
    if not list(sources.glob("*.cs")) and not list(sources.glob("**/*.cs")):
        (sources / "GameBootstrap.cs").write_text(
            "using robot2D;\npublic sealed class GameBootstrap : Entity { }\n", encoding="utf-8")
    atomic_json(root / CONFIG, {"version": 1, "name": name,
                               "sources": os.path.relpath(sources, root)})
    print("Generated scripts project: " + str(project))


def compiler_command(compiler, mono):
    compiler = Path(compiler)
    if compiler.suffix.lower() in (".bat", ".cmd"):
        # Run the managed compiler itself, never pass source names to cmd.exe.
        managed = compiler.parent.parent / "lib/mono/4.5/mcs.exe"
        if not managed.is_file():
            raise ValueError("managed mcs compiler missing: " + str(managed))
        return [str(mono), str(managed)]
    return [str(compiler)]


def build(root, name, core, compiler, mono):
    name = project_name(name)
    root, core = root.resolve(), core.resolve()
    output = root / "assets/scripts/bin" / (name + ".dll")
    stamp = root / ".robot2d/scripts-build.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    # Invalidating first prevents failed builds being mistaken for a previous success.
    compiled_only = output.exists() and not (root / CONFIG).exists() and not any(
        p for p in (root / "assets/scripts").rglob("*.cs") if "bin" not in p.parts and "obj" not in p.parts)
    stamp.unlink(missing_ok=True)
    if compiled_only:
        raise ValueError("legacy project has only a compiled DLL; generate/configure the C# source project before Play")
    output.unlink(missing_ok=True)
    config = root / CONFIG
    if not config.exists():
        generate(root, name, root / "assets/scripts", core)
    data = json.loads(config.read_text(encoding="utf-8"))
    if data.get("version") != 1 or data.get("name") != name:
        raise ValueError("script project configuration does not match project")
    sources = (root / data["sources"]).resolve()
    if not sources.is_dir():
        raise ValueError("script source directory missing: " + str(sources))
    if not core.is_file():
        raise ValueError("ScriptCore is missing: " + str(core))
    refs = sources / "References"
    refs.mkdir(exist_ok=True)
    if core != (refs / core.name).resolve():
        shutil.copy2(core, refs / core.name)
    files = []
    for folder, dirs, names in os.walk(sources, followlinks=False):
        dirs[:] = sorted(d for d in dirs if d.lower() not in {"bin", "obj", "references", ".git"}
                         and not (Path(folder) / d).is_symlink())
        files.extend(Path(folder) / n for n in sorted(names) if n.endswith(".cs") and not (Path(folder) / n).is_symlink())
    if not files:
        raise ValueError("no C# source files: " + str(sources))
    command = compiler_command(compiler, mono)
    # A response file avoids the Windows command line length limit for many sources.
    response = root / ".robot2d/scripts.rsp"
    arguments = ["-target:library", "-sdk:4.8", "-langversion:7.2", "-warnaserror+",
                 "-out:" + str(output), "-r:" + str(core), *map(str, files)]
    if any(any(c in a for c in '\r\n"') for a in arguments):
        raise ValueError("unsupported character in compiler path")
    response.write_text("\n".join('"' + a + '"' for a in arguments), encoding="utf-8")
    print("Building user scripts: " + str(output), flush=True)
    try:
        result = subprocess.run([*command, "@" + str(response)], cwd=root, timeout=120, check=False)
        if result.returncode or not output.is_file():
            raise ValueError("C# compiler failed (exit " + str(result.returncode) + ")")
    except BaseException:
        output.unlink(missing_ok=True)
        raise
    atomic_json(stamp, {"version": 1, "assembly": str(output.relative_to(root)),
                        "core": str(core), "sources": len(files)})
    print("Scripts build succeeded: " + str(output))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-root", type=Path, required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--core", type=Path, required=True)
    parser.add_argument("--sources", type=Path)
    parser.add_argument("--compiler", type=Path)
    parser.add_argument("--mono", type=Path)
    parser.add_argument("--generate", action="store_true")
    args = parser.parse_args()
    try:
        if args.generate:
            generate(args.project_root, args.name, args.sources or args.project_root / "assets/scripts", args.core)
        else:
            if not args.compiler or not args.mono:
                raise ValueError("compiler and Mono paths are required")
            build(args.project_root, args.name, args.core, args.compiler, args.mono)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print("[ScriptBuild] " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
