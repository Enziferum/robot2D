# Core lesson 07 — items and score

Targets `0.8.0-dev` from 8b998ee, with Window metrics baseline 79369b2.
Build within the full checkout (lessons 01/02/06 supply the base application):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-collect-items robot2D-collect-items-test -j 4
ctest --test-dir build -C Release -R '^core-lesson\.items-' --output-on-failure
```

Run `build/examples/collect-items/Release/robot2D-collect-items` (`.exe` on Windows)
from that directory, with matching native runtime DLLs on PATH. CMake stages the
Robot Lab PNG and this example's font/notice into `assets/`. Optional arguments:
PNG path, then font path. Missing font errors include the absolute path.

WASD/arrows move, R starts another round, Escape exits. Five gold items award ten
points each, once. The visible HUD shows score/items and completion. The world
is the fixed 800x600 collision arena. Pickups use final-position AABB overlap after
wall resolution; huge steps or teleports can skip triggers. No persistence,
inventory, physics changes or continuous trigger sweep are included.

The HUD uses Core Font/Text with an explicitly uploaded, single-page ASCII atlas
passed in RenderStates.texture. Text objects are destroyed before Font/context;
textures are released in destroy(). Restart resets state without reloading assets.

Optional hidden-window checks (requires a display and working GL driver):

```sh
cmake -S . -B build -DRB2D_CORE_EXAMPLE_GRAPHICS_TESTS=ON
cmake --build build --config Release --target robot2D-collect-items-graphics-test
ctest --test-dir build -C Release -R '^core-lesson\.items-(render|missing-font)$' --output-on-failure
```

Exit 77 means unavailable GL, not a pass. The render test checks actual HUD glyphs,
score pixel changes, item disappearance/restoration and cleanup. Five headless
scenarios cover state/overlap/reset/focus and a full input-driven route at three
update rates. Manual acceptance: readable HUD, all five items, repeated overlap,
restart before/after completion, focus loss, resize/DPI and real keyboard input.

Code uses the included Zlib license. Robot Lab PNG retains the lesson 01 notice.
SourceSansPro-Regular.ttf is copied unchanged from editor/res/fonts; SHA256:
e0acaced3f5686390c4c2ed8d3b447c725660252d1a20a71fdab5110a435c463.
Its embedded notice names Adobe Systems Incorporated, 2010–2018, Reserved Font
Name "Source". assets/OFL.txt includes that notice and the official OFL 1.1 text
from https://openfontlicense.org/documents/OFL.txt. The font is not under Zlib.
