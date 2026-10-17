---
title: Build from source
description: Requirements and a starting point for a development build.
---

## Requirements

- A C++17 compiler and CMake 3.21 or newer.
- Git, including submodule support.
- An OpenGL 4.5-capable GPU and driver.
- Mono development tools, including `mcs`, with .NET Framework 4.8 reference assemblies. User script builds also need Python 3.
- Platform build dependencies described in the repository and CI configuration.

This is a source-build guide for developers, not an installer. Toolchain setup differs between Windows and Linux. Read the [repository build instructions](https://github.com/Enziferum/robot2D/blob/dev/README.md) and [CI configuration](https://github.com/Enziferum/robot2D/blob/dev/.github/workflows/ci.yml) before choosing a compiler and dependency paths. On Windows, keep native dependencies in the same compiler ABI; do not mix MinGW and MSVC libraries.

## Get the current development branch

```sh
git clone --branch dev --recurse-submodules https://github.com/Enziferum/robot2D.git
cd robot2D
```

For an existing checkout, initialise submodules with `git submodule update --init --recursive`.

## Configure and build

Use a separate build directory. After installing the required native and Mono dependencies and configuring their paths:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target robot2D -j 4
```

Choose a generator that matches your installed toolchain. `robot2D` is the editor target; `robot2D-core` is the core library. The editor depends on the generated `robot2D_ScriptCore` managed API assembly.

Run the built editor from its staged output directory, with its `res` resources and runtime dependencies available. The output path varies by generator and configuration; inspect the build output rather than assuming a universal executable location.

## If configuration fails

Keep the first CMake error and the compiler/generator versions when reporting a problem. Mono paths, reference assemblies, graphics drivers and missing submodules are common environment inputs. The managed API build and required references are defined in [ScriptCore.cmake](https://github.com/Enziferum/robot2D/blob/dev/editor/cmake/ScriptCore.cmake).

The repository README includes build, test and standalone packaging commands. A complete fresh-machine installation walkthrough remains release work.
