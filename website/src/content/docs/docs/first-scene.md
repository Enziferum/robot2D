---
title: Your first scene
description: A small workflow for exploring the editor.
---

## Start small

Create a project using the editor's project workflow, or open an existing development project. Begin with a camera and one textured entity. Save the scene before testing it.

The scene hierarchy lists entities. Selecting one opens its components in the Inspector. Use Transform to position and size it, and Drawable to choose its texture and colour.

## Add a platform and a player

1. Place a wide platform in the scene. Give it a static physics body and a collider.
2. Place a player above it with a dynamic body and a collider matched to the sprite.
3. Keep the camera framing both objects. Start with simple, consistent coordinates and sizes.
4. Add player behaviour through a C# script, then build scripts before testing.

Physics component units and sprite dimensions need to agree. A visually correct sprite is not proof that its collision box is correct; inspect the collider values and test contact with a single platform first.

## Test one change at a time

Press Play and check that the scene starts, then stop before continuing to edit. Check the Console if the script build fails. Do not treat runtime changes as saved scene edits.

The website's Robot Lab screenshot shows the SlateTeal development editor, including the ASSET-01 texture preview and folder-tree work. Your build may differ until those changes are integrated.

A downloadable Robot Lab project and step-by-step asset import tutorial will follow after the demo is packaged and checked. The screenshot alone is not a downloadable demo.
