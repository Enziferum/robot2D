---
title: ECS concepts
description: A reading guide to scenes, entities, components and systems.
---

## The model

A scene brings together entities, their component data and systems. An entity identifies an object; components describe its state; systems operate on the data they need. This separates storage and behaviour without requiring every game object to be a subclass of one large base class.

| Building block                  | What to inspect                                                   |
| ------------------------------- | ----------------------------------------------------------------- |
| `ecs::Scene`                    | Entity creation, scene membership, system registration and update |
| `ecs::Entity`                   | Entity handle and component access                                |
| Component storage               | Component ownership and access rules                              |
| `ecs::System` / `SystemManager` | System execution and scene integration                            |

## Read one real flow

Start at a scene creation call, follow the entity into component storage, then find the system that consumes its data. Keep editor-only state separate from runtime state when reading a Play/Stop transition.

Do not infer that copying an entity handle deep-copies its components. Do not assume an existing handle or C++ reference stays usable after removal or scene clearing. Check the lifecycle contract and tests for the version you use.

## An API in development

Creation, removal, component containers and clone behaviour are active ECS work. This page intentionally explains the concepts without publishing a fixed add/remove/clone recipe that could conflict with that work. Check [ECS-01](https://github.com/Enziferum/robot2D/issues/100) and the related implementation issues before extending it. This documentation does not implement or complete those tasks.

Sources at the documented baseline: [Scene](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Ecs/Scene.hpp), [Entity](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Ecs/Entity.hpp), [System](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Ecs/System.hpp).
