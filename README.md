# Robot2D

<img align="right" src="https://github.com/Enziferum/robot2D/blob/master/logo/logo.png" alt="logo" width="256"/>

\
Windows: [![Build status](https://ci.appveyor.com/api/projects/status/ilga4qquggcsfy65?svg=true)](https://ci.appveyor.com/project/Enziferum/robot2d) \
Mac/Linux: [![Build Status](https://github.com/Enziferum/robot2D/actions/workflows/ci.yml/badge.svg)]
\
Robot2D - C++ 2D Engine. Graphics Api is OpenGL 3.3/4.5.
\
In current version 0.4 support platforms are: Windows / MacOS / Linux.

\
Project's idea - learning, have fun and sometimes make games on it :) 


## Structure
 - Engine - robot2D's core, developing as framework. Can be using standalone like [SFML](https://github.com/SFML/SFML).
 - Editor - robot2D's editor, developing around engine framework. UI is simular to popular projects to be intuitive.
 - Sandbox - robot2D's experiment lab to work on new features or some extra stuff.

Code Guards
-------

[![CodeFactor](https://www.codefactor.io/repository/github/enziferum/robot2d/badge)](https://www.codefactor.io/repository/github/enziferum/robot2d)
[![BCH compliance](https://bettercodehub.com/edge/badge/Enziferum/robot2D?branch=master)](https://bettercodehub.com/)
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
 - Your output will be 'output' folder