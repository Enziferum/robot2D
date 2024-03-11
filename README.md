# Robot2D

<img align="right" src="https://github.com/Enziferum/robot2D/blob/master/logo/logo.png" alt="logo" width="256"/>

\
Windows: [![Build status](https://ci.appveyor.com/api/projects/status/ilga4qquggcsfy65?svg=true)](https://ci.appveyor.com/project/Enziferum/robot2d) \
Linux: [![Build Status](https://github.com/Enziferum/robot2D/actions/workflows/ci.yml/badge.svg)] \
Core tests: [![Build Status](https://camo.githubusercontent.com/937ec0fa06b6039f43e7e0b50502303f71a7726fc7def469aa4bde2916b75711/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f74657374732d3136253230706173736564253243253230312532306661696c65642d637269746963616c)]
\
Robot2D - C++ 2D Engine. Graphics Api is OpenGL 3.3/4.5.
\
In current version 0.7(W.I.P) support platforms are: Windows / MacOS / Linux?.
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
[![BCH compliance](https://bettercodehub.com/edge/badge/Enziferum/robot2D?branch=master)](https://bettercodehub.com/)
-->
-------

## Requirements
   * C++17 compiler
   * CMake
   * Python 3.6 (for automatic build / install deps)

## How-to
 - Install C++17 compiler
 - Install CMake
 - Install Python 3.6+
 - Run as admin:
```shell
  git clone 
  cd robot2D
  python scripts/build_engine.py
```
 - Your output will be inside `output` folder.