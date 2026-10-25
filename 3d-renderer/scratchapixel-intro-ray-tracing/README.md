# Scratchapixel — Introduction to Ray Tracing

Status: **docs only, awaiting approval. No code, no tests, no push yet.**

A minimal ray tracer in **C++**, built from the Build Your Own X
entry for 3D Renderer:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Introduction to Ray Tracing: a Simple Method
  for Creating 3D Images_ (Scratchapixel) —
  https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-ray-tracing/how-does-it-work
- Language in the tutorial: C++ (C++-like pseudocode in places),
  no libraries required for the read chapters.

This is one of 11 separate folders under `3d-renderer/`, one per
BYOX 3D Renderer link. It is not the Avik Das renderer, not a
rasterizer, and not a shared codebase with the sibling folders.
Each folder is docs-first, on its own, as instructed.

## What it will be

- Backward ray tracing: one primary ray per pixel, cast from the
  eye/camera through the centre of that pixel's region of the
  image plane.
- Spheres only, by intersection test (the read chapters' scope).
- Basic shading: a hit pixel is the object's colour scaled by
  light brightness; a shadowed point is black. There is no Phong
  model in the read chapters — no diffuse/specular split, no
  shininess.
- Hard shadows: a shadow ray from the hit point toward each
  light; if anything blocks it, that light contributes nothing.
- Reflection and refraction, in chapter 3 only: a recursive
  trace, with a Fresnel / facing-ratio term deciding how much
  light reflects vs refracts for a glass sphere.
- Output: a PPM image file (ASCII PPM in the tutorial's examples),
  no GUI window, no external image library needed.

## The 3 chapters the docs are based on (verified, read)

1. **How Does it Work** — image plane, perspective projection,
   light and colour, diffuse vs conductor/dielectric materials.
2. **Implementing the Raytracing Algorithm** ("Raytracing in a
   Nutshell") — primary rays through pixel centres, nearest
   intersection, shadow rays.
3. **Writing a Basic Raytracer** — a working example program,
   recursive-trace pseudocode, a few hundred lines total.

## To verify before code (not spec'd here as fact)

- The tutorial's follow-on lessons were NOT fully read: titles
  identified only include "An Overview of the Ray-Tracing
  Rendering Technique", "Generating Camera Rays", and
  "Ray-Tracing a Polygon Mesh", plus snippet-listed topics
  (perspective projection matrix, rays and cameras, rendering
  a triangle/mesh/Utah Teapot, the rasterization algorithm, the
  rendering equation, path tracing, area lights, texturing,
  motion blur, depth of field). None of these is included in
  this build's scope unless a later, read-and-approved revision
  adds it.
- The exact code blocks on the chapter-1 page (the page describes
  a compilable example, but no code block appeared in the text
  fetch used for these docs) — the implementation in code must
  be written from the concepts in SPEC, not copied from a code
  block that was not verified here.
- **C++ toolchain check (pre-code step, not yet done):** whether
  a C++ compiler is available and working in the build VM. These
  docs do NOT claim a C++ toolchain here is verified. TEST_PLAN
  lists this check as step zero; no "tested" claim may be made
  until it passes.

## Planned run (not working yet — code is not written)

```sh
g++ -std=c++17 -O2 -o raytracer main.cpp   # compiler to be confirmed, see above
./raytracer
# writes an output PPM file, e.g. untitled.ppm / minray.ppm in the tutorial's examples
```

Default scene (ours, defined in SPEC): camera/eye at the origin
looking forward, three spheres (one reflective, one glass, one
ordinary), one visible light-source sphere, black background.
The tutorial's own example scenes are described in SPEC, not
copied as code.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — chapters, math, scene, and every deliberate
  divergence / limit
- `docs/DESIGN.md` — classes/structs, tracing loop, file format
- `docs/TEST_PLAN.md` — the tests that must pass before any push
- `docs/TICKETS.md` — build order, following the tutorial's
  chapters, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Divergences and limits, stated up front

- Implementation language is C++, as in the tutorial. No build
  system is prescribed here beyond a single compiler invocation;
  a CMake/Makefile choice is a pre-code decision, not assumed.
- Scene: the default scene in SPEC is ours (positions/colours
  stated there), not a claim to reproduce the tutorial's exact
  example scene values, which were not all verified from the
  text fetch.
- No code from the tutorial is copied in these docs. The design
  follows the tutorial's concepts (primary rays, nearest hit,
  shadow rays, recursive reflection/refraction) in our own
  structure.

## Out of scope, honestly

Phong/diffuse/specular shading, textures, anti-aliasing,
triangle meshes, area lights, path tracing, motion blur, depth
of field, and any follow-on lesson's content. Those are not
claimed; the read chapters' scope is spheres + basic shading +
shadows + chapter-3 reflection/refraction, and so is this
build's.

## Next step

Your approval triggers, in order: toolchain check, then code
(one ticket at a time), test run (including a rendered PPM
checked by pixel), code review against SPEC, then a push
decision — this folder only, as its own directory under
`3d-renderer/`.
