---
title: Graphics and resources
description: Render targets, sprites and the lifetimes behind a visible image.
---

## Four pieces of a drawn image

| Type           | Role                                                                    |
| -------------- | ----------------------------------------------------------------------- |
| `Image`        | CPU-side image data, useful for loading pixels and window icons         |
| `Texture`      | A GPU resource created and released with a compatible active GL context |
| `Sprite`       | Drawable sprite state that refers to a texture                          |
| `RenderTarget` | Drawing destination; `RenderWindow` combines it with a window           |

A sprite does not make a dead texture safe. Keep the texture owner alive while the sprite uses it, and finish any pending draws before releasing the resource.

## Loading a texture

`Texture::loadFromFile(path)` reports success with a bool. Check the result before assigning the texture to a sprite through `setTexture`. Use paths relative to the intended runtime asset directory, not an accidental IDE working directory. For a missing image, report the path and failure at the loading boundary rather than logging the same error every frame.

A correct source path alone is not enough: GPU loading and destruction need the appropriate current graphics context. A background CPU decode is not permission to perform GL work on an arbitrary thread.

## Draw submission

`RenderTarget` exposes `clear`, `draw`, `beforeRender`, `afterRender`, views and batching operations. `draw` can submit batched work; do not assume every call immediately reaches the screen. For several drawables, follow the target's frame lifecycle rather than repeatedly using the convenience `render(singleDrawable)` operation.

A `View` controls the view transform. Physical framebuffer size can differ from logical window size on high-DPI displays. A minimized window can report a zero framebuffer extent, so avoid division by width or height without checking it.

## ResourceHandler

`ResourceHandler<T, ID>` owns resources by unique ID. Loading is synchronous. Duplicate IDs do not replace an existing resource. Insertion preserves pointers and references; `clear()` and destruction invalidate them. Mutating concurrent access needs external synchronization. GPU resource context rules still apply even when a generic handler owns the objects.

These are ownership rules, not a promise of asynchronous streaming, hot reload or automatic lifetime repair.

Sources: [RenderTarget](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Graphics/RenderTarget.hpp), [Sprite](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Graphics/Sprite.hpp), [Texture](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Graphics/Texture.hpp), [ResourceHandler](https://github.com/Enziferum/robot2D/blob/4132acf/core/include/robot2D/Util/ResourceHandler.hpp).
