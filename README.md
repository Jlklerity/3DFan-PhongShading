# Fan Dashboard HUD — Android Programming Quiz 3

A cross-platform (Android / Windows Desktop / WebGL) OpenGL ES 3.0 demo built on top of
the *OpenGL ES 3.0 Cookbook* touch-events framework. A spinning 3D fan renders on a
procedural checkerboard floor, with a screen-space HUD overlaid on top for speed and
zoom control, plus mouse/touch-driven 3D picking to highlight individual fan parts.

## Overview

This quiz extends **Android Programming Quiz 2** by splitting a single `Fan` model into a
proper multi-scene architecture, upgrading lighting to real per-fragment Phong shading,
adding a procedural ground, building a screen-space HUD, and replacing the old
tap-to-toggle gesture with unproject-based 3D picking.

## Architecture

```
Renderer
 ├── Scene3D                 (perspective-projected 3D content)
 │    ├── Fan                (spinning fan: base, pole, hub, 4 blades)
 │    └── Ground             (procedural checkerboard floor)
 └── SceneHUD                (orthographic screen-space overlay)
      ├── 4 control buttons  (+SPD / -SPD / +ZOOM / -ZOOM)
      └── 20-segment speed bar
```

Each frame renders in a fixed pipeline order so the HUD always draws on top of the 3D
scene and never gets occluded by "closer" geometry:

```
Clear framebuffer
  → Render Scene3D            (depth test ON — fan occludes itself correctly)
  → Disable depth test
  → Render SceneHUD            (orthographic, drawn over everything)
  → Re-enable depth test       (restore state for next frame)
  → Swap buffers
```

`Renderer` owns one `Scene3D` and one `SceneHUD`; neither scene talks to the other
directly — `Renderer` routes input and forwards resize/render calls to both.

## Features

### Phong lighting (Fan + Ground)
Both the fan and the checkerboard ground use real per-fragment ambient + diffuse +
specular lighting, computed in view space:
- Normals are transformed by the inverse-transpose of the model-view matrix's upper 3×3
  (`NormalMatrix`), so lighting stays correct as the fan spins and blades rotate.
- Specular uses `reflect(-L, N)` against the view vector, so it produces a true
  view-dependent highlight rather than tracking the diffuse term.
- Ambient intensity is kept low so the diffuse gradient and specular highlight both have
  visible headroom (a saturated ambient term otherwise washes out the specular hot spot).

### Procedural checkerboard ground
The floor is a single large quad; the checkerboard pattern is computed in the fragment
shader from world-space XZ position (`floor()` / `mod()` cell parity) — no texture
asset involved — and lit through the same Phong function as the fan.

### Screen-space HUD
`SceneHUD` builds all geometry (4 buttons + a 20-segment speed bar) in pixel space with
a dedicated orthographic projection, rebuilt on every resize so the layout never drifts,
shrinks, or moves relative to the screen corners regardless of window size or device
rotation.

- **+SPD / -SPD** — adjust fan rotation speed within `[0, 20]`; the live 20-block bar
  fills/empties (green = active, grey = inactive) in lockstep.
- **+ZOOM / -ZOOM** — move the camera within a clamped distance range.
- HUD hit-testing (`SceneHUD::TouchDown`) runs *before* any 3D picking; a button hit is
  consumed and never falls through to the 3D scene, and a miss falls through cleanly.

### 3D picking
Clicking directly on the fan (and missing every HUD button) unprojects the click into a
view-space ray and tests it against each fan part:
- **Base / pole / hub** — ray–sphere test against a cached bounding sphere.
- **Blades** — ray–segment test along each blade's long axis, since a single bounding
  sphere is too imprecise for a long, thin box.

The nearest hit (smallest ray parameter `t`) is highlighted by swapping its color for
amber; clicking the same part again, or clicking empty space, clears the highlight.
Per-part pick volumes are cached in view space every frame (piggybacking on the
model-view matrix already computed for rendering), so the test always reflects the
fan's current spin.

## Controls

| Input | Effect |
|---|---|
| Click/tap `+SPD` | Speed up the fan (max 20) |
| Click/tap `-SPD` | Slow down the fan (min 0) |
| Click/tap `+ZOOM` | Move camera closer (clamped) |
| Click/tap `-ZOOM` | Move camera farther (clamped) |
| Click/tap a fan part | Highlight that part in amber; click again or click empty space to clear |
| Window resize / device rotation | HUD re-layouts; 3D perspective aspect updates |

## Building

| Target | Toolchain |
|---|---|
| Windows Desktop | CMake + GLFW + GLEW |
| Unix Desktop | CMake + GLFW/GLEW (Linux/macOS) |
| Web | Emscripten (GLFW or SDL2 backend, selected at compile time via `USE_GLFW`) |
| Android | Gradle/CMake via NDK, JNI bridge |

All three assignments (Quiz 1, Quiz 2, Quiz 3) build from this same repository; see the
CI/CD workflows under `.github/workflows` for the exact build matrix
(Android / Windows / Ubuntu runners).

## Project layout

```
Scene/
  Fan.h / Fan.cpp           — fan geometry, Phong shading, per-part pick-volume caching
  Ground.h / Ground.cpp     — procedural checkerboard floor
  Scene3D.h / Scene3D.cpp   — owns Fan + Ground, camera, 3D picking ray construction
  SceneHUD.h / SceneHUD.cpp — screen-space HUD geometry, hit-testing, speed bar
  Renderer.h / Renderer.cpp — top-level owner, fixed render/input pipeline
Transform.h / Transform.cpp — matrix-stack utility (provided, unmodified)
ShaderHelper.h               — shader compile/link helper
Platform.h                   — platform detection + GL headers per target
assets/shader/
  FanVertex.glsl / FanFragment.glsl       — Phong shading
  GroundVertex.glsl / GroundFragment.glsl — procedural checkerboard + Phong
  HUDVertex.glsl / HUDFragment.glsl       — flat-colored screen-space quads/triangles
```

## Notes / known fixes applied

- Removed a duplicate, incorrect draw loop in `SceneHUD::Render()` that was slicing
  half-triangles across the wrong vertex offsets and could color the wrong button's
  triangle amber; button rendering now runs through a single correct loop.
- Retired Quiz 2's tap-to-toggle / drag-to-boost gesture code paths entirely (not
  disabled, removed) in favor of the Part 8 picking model described above.

## Bonus features implemented

*(list any bonus items you've added — e.g. smooth zoom/speed easing, press-and-hold
repeat on HUD buttons, a picked-part color swatch on the HUD — here.)*
