# Computer Graphics from Scratch — Build Your Own 3D Renderer

Status: **Code built and tested 2026-10-25: JavaScript, 32/32 tests PASS (both raytracer and rasterizer sub-builds).**
This is one of eleven parallel docs sets under `3d-renderer/`,
one per BYOX 3D Renderer link. This folder covers exactly one
link and nothing else.

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Computer Graphics from Scratch_ by Gabriel
  Gambetta — http://www.gabrielgambetta.com/computer-graphics-from-scratch/introduction.html
  (book, No Starch Press 2021, based on a university course)
- Language in the book: informal pseudocode throughout, plus
  fully working JavaScript implementations/demos that run in a
  browser. This docs set proposes JavaScript for the build, so
  the build matches the book's runnable implementations.
- The book explicitly does NOT teach OpenGL, Vulkan, Metal, or
  DirectX. The only assumed drawing primitive is `PutPixel` —
  set the colour of one pixel.

## Read this first: this is TWO sub-builds, not one small build

The book builds **two complete, fully functional software
renderers**, and the book itself compares them on the same
simple scene:

- **Sub-build A — Part I, Raytracing** (chapters 2–5): a
  raytracer for spheres, with light, shadows, and recursive
  reflections.
- **Sub-build B — Part II, Rasterization** (chapters 6–14): a
  rasterizer for triangle meshes, with lines, filled and shaded
  triangles, perspective projection, clipping, hidden-surface
  removal, shading, and textures.

Their features overlap but are not identical — the book's own
comparison assigns shadows and reflections to the raytracer
and textures to the rasterizer. Building both is **two
projects' worth of work**, not one small build, and these docs
do not pretend otherwise. TICKETS.md keeps Part I and Part II
as separate ticket series, and either part can be developed,
tested, and reviewed on its own. If only one part is ever
built, say which one — do not claim "the book" is done.

## What each sub-build will be

**Part I — Raytracer (spheres only):**
- A canvas, a camera, and rays cast through each pixel
- Spheres as the only geometry (no triangle meshes in Part I)
- Ambient, point, and directional lights, white only, with no
  distance falloff
- Diffuse plus specular reflection, with a specular exponent
  per material
- Shadows and recursive mirror reflections, bounded by a
  recursion depth

**Part II — Rasterizer (triangle meshes):**
- A cube described as vertex and triangle lists, organised as
  Model → Instance → Camera, with 4×4 transforms
- Lines, filled triangles, and shaded triangles
- Perspective projection and a scene/camera transform
- Clipping (instance and triangle level), so a cube behind
  the camera is correctly invisible
- Depth buffering and back-face culling (toggleable in the
  book's demos)
- Flat, Gouraud, and Phong shading
- Textures with perspective-correct interpolation

## What is NOT in this build, honestly

- Refraction — not seen in the verified material
- Anti-aliasing — not seen in the verified material
- Triangle-mesh ray tracing — Part I is spheres only
- Shadows in the rasterizer — the book assigns shadows to the
  raytracer, not to the rasterizer
- Any use of OpenGL, Vulkan, Metal, or DirectX — the book
  teaches software rendering from `PutPixel` up
- Image-file output (PNG/PPM/TGA) — the book's output is a
  browser Canvas and live demos; no file output is stated.
  TEST_PLAN.md proposes a pixel-capture test approach for
  this build, clearly marked as ours, not the book's.

## Output (book vs. ours)

- Book: browser Canvas via `PutPixel`, plus a live JavaScript
  demo per algorithm.
- Ours (proposed, marked as a proposal, not a book claim):
  the same Canvas rendering, plus a test harness that reads
  pixels back from the canvas (see TEST_PLAN.md) so a render
  can be checked headlessly by pixel values instead of only by
  looking at it.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — both renderers, chapter by chapter, and
  every deliberate divergence or open question
- `docs/DESIGN.md` — shared canvas/math core, then one module
  layout per sub-build
- `docs/TEST_PLAN.md` — behaviour and pixel tests, plus the
  toolchain check that must come before any code
- `docs/TICKETS.md` — Part I tickets, then Part II tickets,
  with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Verification status of the source material

Read in full: the Introduction (chapter-by-chapter overview,
both renderers, and the book's own comparison of them).
Read in part: chapter 3 (Light) — opening sections plus its
concluding summary.
For all other chapters, the contents below rest on the
Introduction's own chapter-by-chapter overview plus the
book's live-demo excerpts, and are marked
"per Introduction overview / demo excerpts, to verify before
code" wherever the detail goes beyond that. No chapter's
full internal content is claimed where it was not read, and
no pseudocode from the book is copied verbatim in these docs.

## Next step

User approval first, as for the other ten docs sets. On
approval, the toolchain check in TEST_PLAN.md comes before
any code, then Part I tickets, then (separately) Part II
tickets — each part reviewed against its own SPEC section
before any push is proposed.
