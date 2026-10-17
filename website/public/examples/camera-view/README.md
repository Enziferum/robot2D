# Core lesson 05 — camera and mouse

Inside a configured `0.8.0-dev` robot2D checkout (Core metrics baseline 79369b2):

```sh
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-camera-view robot2D-core-lessons-test
ctest --test-dir build -C Release -R '^core-lesson\.camera$' --output-on-failure
```

Run `build/examples/camera-view/Release/robot2D-camera-view` (`.exe` on Windows).
WASD/arrows move in a 2000x1200 world; left-click places the player under the pointer;
Escape exits. The camera follows, preserves aspect on resize, and stops at world edges.
No external asset is required. Keep lessons 01–04 alongside this directory.
The code uses the included Zlib license. Test actual resize/DPI/mouse behavior manually;
the headless camera test checks mathematical projection, not native input or rendering.
