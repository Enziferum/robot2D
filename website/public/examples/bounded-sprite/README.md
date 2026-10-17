# Core lesson 03 — coordinates, origin and bounds

Build from the robot2D checkout on `0.8.0-dev` (Window metrics baseline 79369b2):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-bounded-sprite robot2D-core-lessons-test
ctest --test-dir build -C Release -R '^core-lesson\.(origins|edges|resize)$' --output-on-failure
```

Run `build/examples/bounded-sprite/Release/robot2D-bounded-sprite` (`.exe` on Windows)
with that output directory as the working directory. CMake copies the lesson 01
Robot Lab PNG next to the executable; an optional argument overrides its path.
WASD/arrows move; Escape exits. Resize to test logical bounds and oversized centering.
The preceding lesson directories are dependencies, not separate installed SDKs.
The source is under the included Zlib license; the PNG retains lesson 01's asset notice.
