# tinyrenderer — Build Your Own 3D Renderer (software rasterizer)

Status: **Code built and tested 2026-10-25: C++, 26/26 pixel tests PASS.**

A software 3D renderer in **bare C++**, built from the Build Your
Own X entry for 3D Renderer:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _How OpenGL works: software rendering in
  500 lines of code_ by Dmitry V. Sokolov (ssloy) —
  https://github.com/ssloy/tinyrenderer/wiki
  (course notes at haqr.eu)
- This is one of 11 separate builds under `3d-renderer/`, one per
  BYOX 3D Renderer link. It is not the Avik Das raytracer in the
  parent `3d-renderer/` docs, and it shares no code with it.

## What it will be

A simplified clone of how OpenGL / Vulkan / Metal / DirectX
work, written from scratch in C++ with no third-party libraries
except a supplied TGA-image class. It is a **software renderer**:
the program generates an image file, it does not use a GPU, and
there is no graphical interface.

- Input: a triangulated 3D model in `.obj` format, plus textures
- Output: a rendered image file, `framebuffer.tga`
- Build: CMake (as in the tutorial repo, e.g.
  `cmake -Bbuild && cmake --build build -j`,
  then run with model and floor `.obj` files as arguments)
- Final code size in the tutorial: about 500 lines, and the
  tutorial states its students typically take 10–20 hours to
  start producing such renderers

## Features (verified from primary text)

- Triangulated meshes (`.obj` input)
- Textures
- Back-face culling, by a signed-area test
- Barycentric interpolation across triangles
- Tangent-space normal mapping (tangent / bitangent / normal
  basis from the UV grid)
- Shadow mapping: a two-pass depth buffer, producing a hard
  binary shadow mask
- Phong named as the local shading model

## To verify before code (not presented as fact)

- The full 14-lesson ordering of the course notes. The wiki
  home page exposed no lesson navigation in the reading pass,
  so no 14-lesson list is claimed here. A secondary 9-lesson
  map exists (framebuffer/pixels, Bresenham lines, triangle
  rasterization/back-face culling, hidden-surface removal,
  perspective projection/viewport, texture mapping, shaders
  as function objects, shadow mapping, ambient occlusion),
  but it is **not primary-verified** and is not used as the
  build order below. TICKETS.md is instead ordered around the
  verified features listed above.
- Whether the course notes' later lessons (indirect lighting,
  toon shading) are in scope. They are NOT in this build's
  scope (see below) unless a fresh, non-rate-limited read of
  the course notes verifies them and the scope is revised.
- The C++ toolchain in this VM (compiler + CMake). This is
  NOT verified. Checking it is a pre-code step in TEST_PLAN.md;
  no build or test result is claimed until that check passes.
- Exact command-line argument order and default output
  resolution of the tutorial program. The tutorial repo's
  example invocation takes model `.obj` files as arguments
  and writes `framebuffer.tga`; this build's CLI (see SPEC)
  is our own design around that behaviour, not a claim about
  the tutorial's exact CLI.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — pipeline, inputs, outputs, and every
  deliberate divergence or unverified item
- `docs/DESIGN.md` — modules, data layout, rendering loop
- `docs/TEST_PLAN.md` — the tests that must pass before any push
- `docs/TICKETS.md` — build order, around verified features,
  with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Out of scope, honestly

Ray tracing, path tracing, a GPU program, real-time display,
and animation. Those are not claimed; this is an offline
software rasterizer. Also out of scope unless verified and
re-scoped: indirect lighting and toon shading (see "to verify"
above). Mesh loading beyond triangulated `.obj` files, and
any image format other than TGA for output, are also out of
scope.

## Next step

Approval triggers, in order: toolchain check (compiler +
CMake), code (one ticket at a time), test run (including a
rendered TGA checked by pixels), code review against SPEC,
then push — this folder only, as its own directory under
`3d-renderer/`, on the user's approval.
