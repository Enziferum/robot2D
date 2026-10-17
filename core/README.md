# robot2D Core

C++17 engine library. Public interfaces and API comments are in
`include/robot2D/`; tests are in `tests/`.

[Core tutorials](https://enziferum.github.io/robot2D/docs/core/)
include complete buildable examples. For dependencies and the editor build,
see the [build instructions](https://github.com/Enziferum/robot2D#build-from-source).

To generate the API reference, install Doxygen and use a dedicated build:

```sh
cmake -S . -B <build-dir> -DRB2D_BUILD_DOCS=ON
cmake --build <build-dir> --target robot2D-core-docs
```

Run these commands from the repository root. The output is
`<build-dir>/docs/core/html/index.html`; documentation generation is optional
and is excluded from the default build.
