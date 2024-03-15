# Robot2D

<img align="right" src="https://github.com/Enziferum/robot2D/blob/dev/logo/logo.png" alt="logo" width="128"/>

\
Windows / Linux: ![Build Status](https://github.com/Enziferum/robot2D/actions/workflows/ci.yml/badge.svg) \
MacOS: Temporary not supported due to OpenGL max version 4.1 \
\
Robot2D - C++ 2D Engine. Graphics Api is OpenGL 4.5 only.
\
In current version 0.8(W.I.P) support platforms are: Windows / Linux / MacOS ?.
\
Primary focus is fully work with project as `Engine` in editor way only. It's means Engine will be private as core 
of editor.
\
Next planning version is 0.8. See `Project 0.8` to check actual tasks.

\
Project's idea - learning, have fun and sometimes make games on it :) 


## Structure
 - Engine - robot2D's core, developing as framework. Can be using standalone like [SFML](https://github.com/SFML/SFML).
 - Editor - robot2D's editor, developing around engine framework. UI is simular to popular projects to be intuitive.
 - Sandbox - robot2D's experiment lab to work on new features or some extra stuff.

Code Guards
-------

[![CodeFactor](https://www.codefactor.io/repository/github/enziferum/robot2d/badge)](https://www.codefactor.io/repository/github/enziferum/robot2d)
<!--- TODO(a.raag) check correctness 
[![BCH compliance](https://bettercodehub.com/edge/badge/Enziferum/robot2D?branch=dev)](https://bettercodehub.com/)
-->
-------

## Requirements
   * C++17 support compiler
   * CMake 3.21+
   * Python 3.6 (for automatic build / install deps)
   * [Mono](https://www.mono-project.com/download/stable/)

## How-to
 - Install C++17 compiler
 - Install CMake
 - Install Python 3.6+
 - Install Mono
 - Run:
```shell
  git clone 
  cd robot2D
  # install deps
  sudo (For *nix) python scripts/install_deps.py
  cmake -B build/ -G "YOUR_GENERATOR" -DCMAKE_BUILD_TYPE=Debug/Release -DRB2D_## (see cmake/Options.cmake)
  # remove target run_all_tests if you don't want build tests
  cmake --build build --config Debug/Release --target robot2D-editor --target run_all_tests -- -j (1 to YOUR_CPU_SUPPORT)
  # if you want run tests
  ctest --output-junit engine_tests.xml --output-on-failure --test-dir build/tests
```
 - Your output will be inside `build` folder.