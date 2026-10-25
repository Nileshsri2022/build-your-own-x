# 3D Soft Engine — Davrous (Build Your Own 3D Renderer, one of 11)

Status: **docs only, no code, no push, no toolchain verification yet.**
This folder is one of eleven parallel docs sets under `3d-renderer/`,
one per BYOX 3D Renderer link. It covers only the Davrous link;
it does not replace, revise, or reuse the Avik Das docs in
`3d-renderer/` itself, and no code is written here yet.

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: "Learning how to write a 3D soft engine from
  scratch in C#, TypeScript or JavaScript", David Rousset —
  https://www.davrous.com/2013/06/13/tutorial-series-learning-how-to-write-a-3d-soft-engine-from-scratch-in-c-typescript-or-javascript/
- Parts in order: 1 Core logic for camera, mesh & device;
  2 Drawing lines and triangles (wireframe); 3 Loading meshes
  exported from Blender in JSON; 4 Filling triangles with
  rasterization + Z-Buffer; 4b Bonus parallelism;
  5 Flat Shading & Gouraud Shading; 6 Textures, back-face
  culling and WebGL.

## What it will be

A CPU-only software 3D engine — a rasterizer, not a ray tracer,
not a raycaster, and not a modeller. Blender is only an external
source of mesh files; the engine itself does not edit models.

- Triangle meshes: a hand-built cube (8 vertices, 12 faces),
  Blender exports in JSON (the Suzanne monkey, a torus demo)
- Perspective camera: position + target, world × view ×
  projection matrices, field of view 0.78, near plane 0.01,
  far plane 1.0
- A back buffer + a per-frame render loop, targeting 60 FPS
- Wireframe, then filled triangles (scanline rasterization),
  then a depth buffer, then flat shading, then Gouraud shading,
  then textures, then back-face culling
- Animated output: a rotating mesh in a window / on a canvas,
  not a still image file

## Languages and output

The tutorial gives the same engine in parallel in three
languages: C#, TypeScript, and JavaScript. The docs keep that
parallel structure: one engine, three implementations, same
behaviour tests (see TEST_PLAN).

Output, by language, as taught:

- C#: a writable bitmap shown in a XAML image control,
  refreshed on every frame
- TypeScript / JavaScript: an HTML5 canvas, driven by
  `requestAnimationFrame`, reading and writing its pixel buffer

There is no image-file output (no PNG, PPM, or TGA) in this
engine. Verification is therefore by captured frames and by
pixel checks on the back buffer, not by a file on disk.

## Deliberate divergence: modern tooling, same algorithm

The tutorial's tooling is 2013-era: Windows 8, Visual Studio
2012, TypeScript 0.9, with SharpDX for C# and a small
`babylon.math` library for TypeScript/JavaScript. Those exact
toolchains are not proposed here.

Proposed instead, to be confirmed by the pre-code toolchain
check in TEST_PLAN (no toolchain is claimed verified yet):

- C#: a current .NET SDK and a current windowing/bitmap stack
  that can present a writable pixel buffer per frame
- TypeScript / JavaScript: a current TypeScript compiler and a
  plain HTML5 canvas page, with no framework dependency

Only the tooling changes. The algorithm — camera, mesh,
scanline rasterization, depth buffer, flat/Gouraud shading,
texture mapping, back-face culling — stays as taught. If the
toolchain check finds that a 2013-era dependency cannot be
replaced without changing behaviour, that is a blocker to
record in TEST_PLAN, not a silent substitution.

Matrix theory is treated in the tutorial as a black box:
the reader is sent to external articles on world, view, and
projection matrices and is told to use a matrix library
(SharpDX / `babylon.math`) rather than derive the maths.
These docs record that, rather than pretending the tutorial
teaches the matrix maths itself (see SPEC §7 and GLOSSARY).

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — parts, scene, camera, rendering stages,
  and every deliberate divergence from the tutorial
- `docs/DESIGN.md` — objects, rendering loop, back buffer
- `docs/TEST_PLAN.md` — the behaviour/pixel tests that must
  pass before any code push; includes the toolchain check
- `docs/TICKETS.md` — build order, following the tutorial's
  parts, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Coverage honesty — read this before code

Parts 1–3 were read in full. Parts 4–6 were read only in part
in the review pass that produced these docs. Anything below
that was not seen in full is marked in SPEC and TEST_PLAN as
**to verify before code** and is not asserted as fact:

- Part 4: the Z-buffer prose and the remainder of the
  rasterization code beyond the scanline core
- Part 5: the middle of the Gouraud shading code
- Part 6: texture-interpolation code detail, the back-face
  culling code, and the WebGL comparison section

No tutorial code is copied verbatim in these docs. Class and
function names that the tutorial itself uses (Camera, Mesh,
Device, `ProcessScanLine`, `ComputeNDotL`) are named so the
design is traceable, but the implementations here are to be
written from the described behaviour, not pasted from the
tutorial.

## Out of scope, honestly

Spheres as primitives, shadows, reflections, refraction,
specular / Phong highlights, anti-aliasing inside the engine
itself, triangle clipping (only screen-space point clipping
was seen), HTML5/Web Workers parallelism, and a WebGL engine
of our own. The tutorial's final WebGL section loads the same
JSON mesh into the existing Babylon.js WebGL engine for
comparison only — the reader does not build a WebGL engine,
and neither does this project. The only anti-aliasing seen
is GPU "hardware scaling": rendering at a lower back-buffer
resolution and letting the GPU stretch it to the display
size. That is not an engine anti-aliasing feature and is not
claimed as one.

## Next step

User approval only. After approval, in order: toolchain check
(TEST_PLAN §0), code (TDD, one ticket at a time), behaviour /
pixel tests, code review against SPEC, then a separate push
approval. No code and no push are authorised by these docs
alone.
