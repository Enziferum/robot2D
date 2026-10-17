# Core lesson 01: first sprite

These files are the finished example for the website lesson. They are built
**inside a robot2D source checkout**, not against an installed SDK.

From the repository root, with the normal native dependencies/submodules ready:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-first-sprite -j 4
cd build/examples/first-sprite/Release
./robot2D-first-sprite
```

On Windows use `./robot2D-first-sprite.exe`. Native DLLs must be discoverable by
the matching toolchain runtime (for example run from an MSYS2 MinGW64 terminal).
For an existing configured build, reuse its directory and dependency options.
The executable is placed in `<build>/examples/first-sprite/<configuration>/`.
CMake copies `assets/` beside it after linking. Keep that working directory, or
pass an explicit PNG filename as the first argument. Rebuild after changing code;
a texture-only edit can be copied into the output `assets/` directory manually.

An empty-window starting point is `../core-window.cpp`; the lesson walks from
that application skeleton to `FirstSprite.hpp` and `main.cpp` here. The published
code and CMake build use the same files, without a second handwritten code listing.
The CMake snippet assumes the root has already defined `robot2D-core`.

Optional automated graphics checks (a hidden GLFW window, no UI automation):

```sh
cmake -S . -B build -DRB2D_BUILD_CORE_EXAMPLES=ON -DRB2D_CORE_EXAMPLE_GRAPHICS_TESTS=ON
cmake --build build --config Release --target robot2D-first-sprite-test -j 4
ctest --test-dir build -C Release -R '^core-lesson' --output-on-failure
```

A graphics context is required, even though the window is hidden. On Linux run
CTest under Xvfb if needed. Return code 77 is a skipped unavailable-context check,
not successful rendering. The tests exercise the published application's
setup/render/close/destroy methods; they do not automate the editor or drive the
Engine event loop.

## Asset provenance

`assets/robot.png` is the original transparent Robot Lab `Player.png`, generated
for this project with image_gen during platformer demo preparation and reused
with the project owner's request. It is example artwork, not an engine logo or
a third-party asset pack. No third-party attribution or license is asserted.
The C++ source follows the repository Zlib license; see the included LICENSE.txt.
PNG SHA-256: `8e0e1a452183f0b6346fa8f972776e83daf221d9bfd658ebbbc7f20fcdc191f9`.
