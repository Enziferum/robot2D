"""Exercise the actual relocated standalone executable, never user projects."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=["inputs", "runtime"])
    parser.add_argument("--player", type=Path, required=True)
    parser.add_argument("--fixture", type=Path, required=True)
    parser.add_argument("--presentation", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="player-fixture-", dir=Path.cwd()) as temporary:
        root = Path(temporary)
        game = root / "source"
        # Copy the player template (native dependencies + Mono), not the editor.
        shutil.copytree(args.player.parent, game)
        fixture_env = os.environ.copy()
        fixture_env["PATH"] = str(args.player.parent) + os.pathsep + fixture_env.get("PATH", "")
        subprocess.run([str(args.fixture), str(game)], env=fixture_env, check=True, timeout=20)
        moved = root / "Moved game & spaces"
        if os.name == "nt": moved = root / "Перенесенная игра & spaces"
        shutil.copy2(args.presentation, game / args.presentation.name)
        for attempt in range(20):
            try:
                game.rename(moved)
                break
            except PermissionError:
                if os.name != "nt" or attempt == 19:
                    raise
                # Windows virus scanners may briefly hold newly copied executables.
                time.sleep(0.1)
        player = moved / args.player.name
        cwd = root / "unrelated cwd"
        cwd.mkdir()
        environment = os.environ.copy()
        for name in ("MONO_PATH", "MONO_CFG_DIR", "MONO_GAC_PREFIX", "MONO_ENV_OPTIONS"):
            environment.pop(name, None)
        if os.name == "nt":
            environment["PATH"] = str(Path(os.environ["SystemRoot"]) / "System32")
        report = root / "report.txt"
        environment["RB2D_PLAYER_TEST_REPORT"] = str(report)

        def run(expected, *extra):
            report.unlink(missing_ok=True)
            result = subprocess.run([str(player), "--hidden", *extra], cwd=cwd, env=environment,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=25)
            output = result.stdout.decode("utf-8", errors="replace")
            if result.returncode != expected:
                raise AssertionError(f"exit {result.returncode}, expected {expected}:\n{output}")
            return output

        manifest = moved / "game-launch.yaml"
        original = manifest.read_text(encoding="utf-8")
        run(0, "--check")
        run(0, "--manifest", str(manifest), "--check")
        for old, new in [("Version: 1", "Version: 99"),
                         ("assets/scenes/main.scene", "../escaped.scene"),
                         ("Width: 640", "Width: 0"),
                         ("runtime/robot2D_ScriptCore.dll", "runtime/missing.dll"),
                         ("MonoEtc: runtime/mono/etc", "MonoEtc: absent"),
                         ("assets/scenes/main.scene", str((moved / "assets/scenes/main.scene").as_posix()))]:
            manifest.write_text(original.replace(old, new), encoding="utf-8")
            run(1, "--check")
        manifest.write_text(original, encoding="utf-8")
        bcl = moved / "runtime/mono/lib/mono/4.5/mscorlib.dll"
        bcl.rename(bcl.with_suffix(".hidden"))
        run(1, "--check")
        bcl.with_suffix(".hidden").rename(bcl)
        try:
            external = root / "external.scene"
            external.write_text("outside package", encoding="utf-8")
            link = moved / "assets/scenes/linked.scene"
            link.symlink_to(external)
        except OSError:
            pass  # Windows symlink creation may require developer mode.
        else:
            manifest.write_text(original.replace("assets/scenes/main.scene", "assets/scenes/linked.scene"), encoding="utf-8")
            run(1, "--check")
            manifest.write_text(original, encoding="utf-8")
        if args.mode == "runtime":
            presentation = subprocess.run([str(moved / args.presentation.name), str(manifest)], cwd=cwd,
                                          env=environment, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=25)
            if presentation.returncode != 0:
                raise AssertionError("Presented frame test failed:\n" + presentation.stdout.decode("utf-8", errors="replace"))
            output = run(0)
            if not report.exists() or report.read_text() != "PASS 3 12":
                raise AssertionError("Managed gameplay never completed:\n" + output)
            if "Runtime started" not in output or "Runtime stopped; exit_code=0" not in output:
                raise AssertionError("Runtime lifecycle log missing:\n" + output)
            if "[PlayerStartup] stage='first_frame_presented'" not in output:
                raise AssertionError("First frame timing boundary missing:\n" + output)
            font = moved / "assets/fonts/hud.ttf"
            font.rename(font.with_suffix(".hidden"))
            output = run(1)
            if "Missing font" not in output: raise AssertionError(output)
            font.with_suffix(".hidden").rename(font)
            assembly = moved / "assets/scripts/bin/Arena.dll"
            assembly_bytes = assembly.read_bytes()
            assembly.write_bytes(b"invalid assembly")
            run(1)
            assembly.write_bytes(assembly_bytes)
            scene = moved / "assets/scenes/main.scene"
            scene.write_text("SceneVersion: 999\n", encoding="utf-8")
            run(1)
        print("player " + args.mode + " passed")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
