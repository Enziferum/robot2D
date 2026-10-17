---
title: C# scripting
description: How managed gameplay code connects to the editor.
---

robot2D uses Mono, a .NET Framework 4.8 profile and C# 7.2 for gameplay scripts. This is not the modern .NET SDK workflow.

## Script project and build

The development editor creates a starter script project when first building a new project. **Project → Generate Project** selects a source directory and generates the managed project. **Project → Build Scripts** builds without entering Play.

Play first checks the script build and the scene's script classes. A failed build keeps the editor in Edit mode and provides diagnostics. Read the Console and fix the first compiler error before retrying.

Source files are discovered recursively under the configured directory. `bin`, `obj`, `References` and symbolic links are excluded. The supported build path uses the engine API and standard framework references; arbitrary NuGet packages and custom MSBuild targets are outside this initial contract.

## Explore the API

Entity scripts use the `robot2D` namespace and derive from `Entity`. The managed source is the authoritative API reference:

- [Entity and component access](https://github.com/Enziferum/robot2D/blob/dev/editor/scripting/robot2D-ScriptCore/robot2D-ScriptCore/Scene/Entity.cs)
- [Components](https://github.com/Enziferum/robot2D/blob/dev/editor/scripting/robot2D-ScriptCore/robot2D-ScriptCore/Scene/Component.cs)
- [Input](https://github.com/Enziferum/robot2D/blob/dev/editor/scripting/robot2D-ScriptCore/robot2D-ScriptCore/Input.cs)

## Know the boundaries

Managed API reload happens in Edit mode. This guide does not promise hot reload during Play. See the repository README for [standalone packaging](https://github.com/Enziferum/robot2D#standalone-player); the editor's managed API build is defined in [ScriptCore.cmake](https://github.com/Enziferum/robot2D/blob/dev/editor/cmake/ScriptCore.cmake).
