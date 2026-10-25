# Ray Tracing in One Weekend — Build Your Own 3D Renderer

Status: **Code built and tested 2026-10-25: C++ Book 1, all tests PASS, final 400x225 render done.**
This folder is one of the parallel docs sets under `3d-renderer/`,
one per BYOX 3D Renderer link. It covers exactly one link, not
all 11, and it is not a replacement for the Avik Das docs in the
parent `3d-renderer/` folder.

## Source

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Ray Tracing in One Weekend_, by Peter
  Shirley — https://raytracing.github.io/books/RayTracingInOneWeekend.html
- Language of the tutorial: **C++** (standard library only in
  the listings seen).
- The tutorial's own description, from its Overview: what it
  builds is technically a **path tracer**, and a fairly general
  one — not a rasterizer, not a Phong-shaded renderer.

## What it will be

A from-scratch path tracer in C++, built chapter by chapter,
that renders a scene of spheres to an ASCII PPM image on
stdout. No external graphics API, no image library, no GUI
window.

- Rays from a camera through each pixel, into a list of
  hittable spheres; the nearest hit wins.
- Sky-gradient background (white to blue) where no sphere is hit.
- Materials: Lambertian diffuse, metal (mirrored and fuzzy),
  dielectric (glass: refraction, total internal reflection,
  Schlick approximation, hollow glass sphere).
- Jittered multi-sample anti-aliasing: several random samples
  per pixel, averaged.
- Positionable camera, and defocus blur via a thin-lens camera
  model.
- A final render: a large ground sphere plus many small random
  spheres and three large feature spheres.

## What it will NOT be (honest scope)

- No Phong shading. This book does not use a Phong model;
  do not claim it.
- No explicit light sources and no shadow rays. The only
  shadow-related mechanism in Book 1 is the shadow-acne fix:
  hits are accepted only from a small positive distance
  (0.001 in the listings seen), so a ray does not immediately
  re-hit the surface it left. That is not a shadow feature.
- No triangles, meshes, or textures in Book 1. Those belong to
  Books 2 (The Next Week) and 3 (The Rest of Your Life), which
  are out of scope here.
- No PNG, TGA, Canvas, or window output. Book 1 output is
  ASCII PPM (`P3`) written to stdout.

## Planned run (not working yet — code is not written)

```sh
# Build step to be confirmed at code time; the C++ toolchain
# in this VM is NOT yet verified (see TEST_PLAN, pre-code step).
./raytracer > image.ppm
```

Default render parameters are those of the book's final scene
as described in SPEC: 16:9 aspect, 400 pixels wide in the
listings seen, 100 samples per pixel in the main program
seen, recursion depth limit 50 in the main program seen
(10 in the earlier listings).

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — chapters, math, scene, and every deliberate
  boundary of Book 1
- `docs/DESIGN.md` — classes, rendering loop, file format
- `docs/TEST_PLAN.md` — the pixel-based tests that must pass
  before any push
- `docs/TICKETS.md` — build order, following the book's
  chapters, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Reading coverage — stated honestly

Chapter list verified via the book's table of contents.
Body text was directly read for the tail of Chapter 8
(Antialiasing) through most of Chapter 9 (Diffuse Materials).
All other chapters are known from the table of contents and
from headings located in the book page, **not** from a full
body read, and are marked "per TOC, to verify before code"
in SPEC where that matters. Nothing in those chapters is
described here beyond what the TOC/headings establish, and
no code from the book is copied verbatim anywhere in these
docs — the design below is paraphrased in our own structure
and wording.

## Next step

Your approval, then: verify the C++ toolchain, verify the
per-TOC chapters against the book text, code ticket by
ticket with tests, review against SPEC, and only then push
— this folder only, as part of the 3d-renderer docs push,
on your per-push approval.
