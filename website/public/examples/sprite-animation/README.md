# Core lesson 04 — atlas animation

Inside a configured `0.8.0-dev` robot2D checkout (Core metrics baseline 79369b2):

```sh
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-sprite-animation robot2D-core-lessons-test
ctest --test-dir build -C Release -R '^core-lesson\.animation$' --output-on-failure
```

Run `build/examples/sprite-animation/Release/robot2D-sprite-animation` (`.exe` on Windows).
WASD/arrows move, Escape exits. No external assets are required: BeaconAtlas.hpp
creates the four-cell teaching texture once in setup. It is not Robot Lab character art.
Keep lessons 01–03 alongside this directory. This code and procedural asset use the
included Zlib license. Native dependencies are the same as lesson 01.
