---
title: Core overview
description: The C++ library behind robot2D, and where to start reading it.
---

Core is the C++ library underneath the editor. Its CMake target is `robot2D-core`; the editor target is `robot2D`. Core supplies windowing, events, graphics, resources and ECS building blocks. Editor panels, project workflows and C# integration live above this layer.

This is an introductory map, not a frozen 1.0 API reference. The lifecycle notes below were checked against **dev commit 4132acf**. Older development branches can have different window constructors. Source links are pinned so you can see the contract being described.

## Choose a path

| What you want to understand                         | Start here                                    |
| --------------------------------------------------- | --------------------------------------------- |
| Engine startup, a window, input and shutdown        | [Application and events](application-events/) |
| Sprites, textures, draw calls and resource lifetime | [Graphics and resources](graphics-resources/) |
| Scenes, entities and systems                        | [ECS concepts](ecs/)                          |

## Source map

| Area                 | Responsibility                                            | Main entry points                                              |
| -------------------- | --------------------------------------------------------- | -------------------------------------------------------------- |
| Application / Engine | Application hooks and the engine-owned loop               | `Application.hpp`, `Engine.hpp`, `core/src/Engine.cpp`         |
| Core                 | Desktop lifetime, windows, typed events, time and vectors | `Core/DesktopRuntime.hpp`, `Core/Window.hpp`, `Core/Event.hpp` |
| Graphics             | Render targets, sprites, textures, views and drawing data | `Graphics/RenderWindow.hpp`, `Graphics/RenderTarget.hpp`       |
| Util                 | Resource ownership and diagnostics                        | `Util/ResourceHandler.hpp`, `Util/Logger.hpp`                  |
| Ecs                  | Scene composition and system execution                    | `Ecs/Scene.hpp`, `Ecs/Entity.hpp`, `Ecs/System.hpp`            |

Headers are under [core/include/robot2D](https://github.com/Enziferum/robot2D/tree/4132acf/core/include/robot2D). Implementations are under [core/src](https://github.com/Enziferum/robot2D/tree/4132acf/core/src). Inspect the implementation and tests when a lifetime or error-handling detail matters.

## Build and check

From a configured repository checkout:

```sh
cmake --build build --target robot2D-core -j 4
```

Core tests require `RB2D_BUILD_CORE_TESTS=ON`; build `robot2D-core-tests`, then run CTest in `build/core/tests`. See the repository's [testing guide](https://github.com/Enziferum/robot2D/blob/dev/docs/editor-testing.md) for editor headless versus graphics coverage. A compiled example is not proof that its window or rendering was exercised.

For engine users starting with the editor, the [Robot Lab learning path](../learn/) is a better first stop than extending Core.

Start with [Lesson 01: your first sprite](first-sprite/) for a buildable window, PNG and sprite example.
