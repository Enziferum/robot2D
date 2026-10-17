# Core lesson 02: keyboard movement

This extends lesson 01 inside a robot2D source checkout. Keep both directories:
`first-sprite/` supplies `FirstSprite.hpp`, resource lifetime/rendering and the
original Robot Lab PNG; `keyboard-movement/` adds input state and movement.
There is one shared texture, not a second copy of the artwork.

From the repository root, with the normal native dependencies/submodules ready:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRB2D_BUILD_CORE_EXAMPLES=ON
cmake --build build --config Release --target robot2D-keyboard-movement robot2D-keyboard-movement-test -j 4
ctest --test-dir build -C Release -R '^core-lesson.keyboard' --output-on-failure
cd build/examples/keyboard-movement/Release
./robot2D-keyboard-movement
```

On Windows use `./robot2D-keyboard-movement.exe` with matching runtime DLLs on
PATH (for example an MSYS2 MinGW64 terminal). For an existing configured build,
reuse its directory/toolchain/dependency options. The root config still checks
repository dependencies; this is not a standalone installed SDK project.

CMake copies the first lesson's `assets/` beside the executable. Run from that
output directory, or supply an explicit PNG path as the first argument. After
changing only a texture, copy it into the staged `assets/` folder or relink to
repeat the post-build copy. See `../first-sprite/README.md` for asset provenance.
C++ sources use the included Zlib license; the shared PNG remains Robot Lab art.

Controls: WASD or arrows; Escape or window close exits. Speed is 180 world units
per second. Diagonals are normalized, opposing keys cancel, repeat events do not
move the sprite, and focus loss clears held keys. After focus returns, release
and press the movement keys again. There are no world bounds or collisions.

Current Core baseline: 4132acf. Engine calls `update()` with a fixed timestep
`1 / maxFrameRate`, possibly several times between renders. This parameter is
in seconds. It is not a newly measured render delta or a guarantee of final
scheduler semantics. The formula remains `direction * speed * deltaTime`.

Five headless tests exercise the actual published application: hold/release and
repeat, equal elapsed time with different step subdivisions, diagonal/opposite
and alias keys, focus loss/regain, typed events/short taps/Escape/close. No setup,
texture loading or GL context is used by these motion tests; lesson 01 has separate
graphics checks. They do not simulate physical OS keyboard input.
