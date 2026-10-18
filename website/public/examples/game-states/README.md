# Core lesson 08 — playing, paused and won

Targets the `0.8.0-dev` Core API from baseline f3f53ec. Build within the full checkout:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-game-states robot2D-game-states-test -j 4
ctest --test-dir build -C Release -R '^core-lesson\.states-' --output-on-failure
```

Run `build/examples/game-states/Release/robot2D-game-states` (`.exe` on Windows)
from that directory, with matching runtime DLLs on PATH. CMake stages the PNG and
lesson 07's font/OFL into assets/. Optional arguments: PNG path, then font path.
Source dependencies are lessons 01/02/06/07; the example is not an installed SDK.

- WASD/arrows: move during Playing.
- P: pause/resume; repeat is ignored. Won cannot resume with P.
- R: new round from any phase while focused; repeat is ignored.
- Escape/close: exit from every phase.

Focus loss and minimize notifications pause Playing. Returning/restoring does not
resume automatically. Pause and victory freeze movement and pickups, while the
event pump and drawable rendering continue. Transitions clear held directions:
resume/restart require a fresh press. The HUD shows the state and score. Restart
restores items/spawn without reloading resources. There is no catch-up time, scene
manager, new Engine scheduler or pause menu UI.

Optional GL check (working display/context required):

```sh
cmake -S . -B build -DRB2D_CORE_EXAMPLE_GRAPHICS_TESTS=ON
cmake --build build --config Release --target robot2D-game-states-graphics-test
ctest --test-dir build -C Release -R '^core-lesson\.states-render$' --output-on-failure
```

Six headless cases cover transitions, simulation/input gating, focus/minimize,
victory, restart and exit. The graphics check runs the published application in a
hidden window: frozen world/changed HUD pixels, victory frame, restart and cleanup.
Exit 77 means unavailable GL, not a pass. Perform visible window/input/DPI acceptance
manually; no UI automation is required.

Code uses the included Zlib license. PNG provenance and the font's OFL notice stay
with the assets in lessons 01 and 07. Earlier examples expose small protected
input/HUD helpers; their own behavior remains unchanged.
