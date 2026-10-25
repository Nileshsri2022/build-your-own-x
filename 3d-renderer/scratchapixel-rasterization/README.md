# Scratchapixel Rasterization — Build Your Own 3D Renderer

Status: **Code built and tested 2026-10-25: C++, 26/26 pixel tests PASS.**

A software triangle rasterizer in **C++**, built from one of the
11 links in the Build Your Own X "Build your own 3D Renderer"
section:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Rasterization: a Practical Implementation_
  by Scratchapixel —
  https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/overview-rasterization-algorithm
- This is one of 11 parallel docs sets under `3d-renderer/`,
  one per BYOX 3D Renderer link. It is a separate project from
  the Avik Das raytracer docs in `3d-renderer/` itself, and from
  the other 10 link folders. Nothing here is shared with them
  except the parent directory.

## What it will be

A program that turns a 3D scene made of triangles into a 2D
image, by rasterization — not by ray tracing. The two
algorithms solve the same visibility problem in opposite
orders:

- Ray tracing is image-centric: for each pixel, shoot a ray
  and find the nearest object it hits.
- Rasterization is object-centric: for each triangle, project
  its vertices onto the screen, then fill the pixels its
  projected 2D shape covers.

This build follows the rasterization side. It renders a
triangle mesh to a PPM image file. It does not open a window:
the lesson saves the frame buffer to disk precisely because
there is no cross-platform way to display an image directly
from a plain C++ program.

## What it will NOT be — stated honestly

This lesson offers **no shading solutions**. That is the
lesson’s own statement, not a gap in these docs. What the
build produces is flat triangle colour, or colour interpolated
from vertex colours. There is:

- No Phong, diffuse, or specular lighting model
- No shadows, reflections, or refraction
- No texture sampling result (texture/st coordinates exist
  as data and are interpolated perspective-correctly, but no
  sampled texture is shown as an output)
- No anti-aliasing in the shipped code (sub-pixel sampling
  is discussed and set as an exercise, not presented as
  shipped code)

If a shaded, lit renderer is what is wanted, that is a
different link’s project (for example the Avik Das raytracer
or Ray Tracing in One Weekend), not this one.

## Planned run (not working yet — code is not written)

```sh
# Build tool / compiler to be confirmed before code (see TEST_PLAN):
# the C++ toolchain in this VM is NOT verified yet.
c++ -o rasterizer rasterizer.cpp
./rasterizer out.ppm
```

Default output (ours, defined in SPEC): a PPM file of a
single triangle scene at a small fixed size, used as the
first pixel-checkable render.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — chapters, algorithm, scene, and every
  deliberate divergence and every unverified item
- `docs/DESIGN.md` — modules, pipeline stages, data layout
- `docs/TEST_PLAN.md` — the pixel-based tests that must pass
  before any push
- `docs/TICKETS.md` — build order, following the tutorial’s
  chapters, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Unverified item, flagged in every doc that touches it

- **Chapter Four, "The Depth Buffer"**: referenced by title
  only in the chapters that were read. Its exact URL and
  contents were NOT opened. Do not treat any detail of that
  chapter as fact until it is verified before code. The
  z-buffer behaviour in SPEC/DESIGN is therefore written
  only from what the overview and concluding chapters state
  (a float depth array, nearest wins), and is marked as
  such wherever it appears.

## Out of scope, honestly

Ray tracing, Phong or any lighting model, shadows,
reflections, refraction, texture sampling, mesh loading
beyond the lesson’s own triangle-vertex input, animation,
and any GPU API (OpenGL/Vulkan/Metal/DirectX). The lesson
is about how the GPU pipeline’s visibility algorithm works,
not about writing a GPU program.

## Next step

The user’s approval, after all 11 docs sets are pushed,
triggers, in order: toolchain check, code (one ticket at a
time), test run (including PPM pixel checks), code review
against SPEC, then push — this folder only, as its own
directory under `3d-renderer/`.
