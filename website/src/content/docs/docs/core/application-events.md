---
title: Application and events
description: Engine-owned startup, desktop lifetime and typed event handling.
---

## The high-level path

Derive from `robot2D::Application` and give it to `runEngine`, usually through `ROBOT2D_MAIN`. The application implements `setup()` and `render()` and can override event, update, GUI and shutdown hooks. `m_window` is supplied by Engine; it is borrowed, not something the application deletes.

The [small example](../../../examples/core-window.cpp) opens a plain window through this high-level API. It contains no game assets or scripting setup. It is a learning fragment to add to a target linked against `robot2D-core`, not a separately packaged game.

```cpp
void handleEvents(const robot2D::Event& event) override {
    if(event.getIf<robot2D::Event::Closed>())
        m_running = false;
}
```

`getIf<Payload>()` returns a pointer only for a matching payload. Check it before accessing data. Event values own their payloads; reception sequence, frame and time are metadata, not a guarantee about the original physical input time.

## Desktop and window lifetime

At **4132acf**, `Engine::run` owns a `DesktopRuntime`, then a `RenderWindow`, then the application lifetime. Applications using the high-level Engine facade do not need to create a second desktop runtime.

For low-level integration:

- `DesktopRuntime` owns or borrows the native desktop backend and must outlive its windows and cursors.
- Create and use it on its main thread. It cannot be copied or moved; one runtime may be active.
- `Window` takes `DesktopRuntime&`, and is also non-copyable and non-movable.
- `getRaw()` is a borrowed native handle. Do not destroy it or replace backend callbacks through it.
- Select the correct GL context with `makeCurrent()` before graphics work and resource teardown.

These constructors differ from older dev branches. Match your source commit before copying a low-level window example.

## A frame in Engine

The desktop pumps native events. The window drains an ordered owning batch through `takeEvents(frame)`, and Application receives `handleEventFrame`. The current loop then processes accumulated updates, GUI update and rendering. Do not add another native event pump inside every entity or panel.

This describes the current implementation; it does not promise the final physics scheduler or a stable fixed-update gameplay contract. Those are separate development tasks.

## Shutdown

Release graphics-dependent application resources while their GL context still exists. At the checked commit, Engine makes its window current for application teardown before destroying the window and desktop. Avoid keeping resource owners in longer-lived globals.

Sources: [Application](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Application.hpp), [Engine loop](https://github.com/Enziferum/robot2D/blob/4132acf/core/src/Engine.cpp), [DesktopRuntime](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Core/DesktopRuntime.hpp), [typed Event](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Core/Event.hpp).

Continue with [Lesson 02: keyboard movement](../keyboard-movement/) for typed input events, held state and time-based movement.
