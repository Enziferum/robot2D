---
title: Reading a scene
description: Learn to identify a camera, a player and a platform in Robot Lab.
---

**Goal:** explain how a visible game scene connects to its hierarchy and Inspector. You do not need a downloadable project for this introduction; use the actual editor screenshot below. This is a reading exercise, not a tested installation walkthrough.

<img src="../../../media/robot-lab-editor.webp" width="2559" height="1439" loading="lazy" style="max-width:100%;height:auto" alt="Robot Lab running inside the editor, with Player selected, components shown and textures visible in Assets." />

## 1. Find the same object in two places

The yellow robot appears in the Game View. Its `Player` entity is selected in the hierarchy on the left. The Inspector on the right describes that selected entity. The hierarchy is the scene's organisation; the image in the middle is one view of its current state.

**Check:** you should be able to point to the robot, the selected Player entry and the Inspector name, and explain why they refer to the same scene object.

## 2. Separate appearance from behaviour

A texture gives the player its visible artwork. Transform places and sizes it. The Script component names its C# behaviour. Physics2D and Collider2D describe the physical body and collision shape. A sprite can look right while its collider is the wrong size.

**Check:** identify which component you would inspect for a wrong picture, a wrong position and a collision mismatch. These are different questions.

## 3. Read one platform

Find a landing-platform entity in the hierarchy, then locate its visible counterpart. Platforms define the route, but their textured image is not by itself a physics surface. In a project, inspect the body and collider as well.

**Exercise:** sketch a three-platform route on paper. Mark a start, a safe landing on each platform and an exit. List the separate visual and physical data you would need to represent it.

## What comes next

The reproducible lesson will supply a starting project and the exact engine commit. It will then guide you through placing a platform, sizing its collider and checking it in Play. Until those files are published and tested, this article makes no promise of a one-click tutorial download.
