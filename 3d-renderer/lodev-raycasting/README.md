# Lodev Raycasting — Build Your Own 3D Renderer (link 3 of 11)

Status: **docs only, awaiting further instruction.
No code, no tests, no code push yet.**

A first-person raycaster in the style of Wolfenstein 3D,
built from one of the 11 links in the Build Your Own X
"Build your own 3D Renderer" section:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Raycasting_, Lodev —
  http://lodev.org/cgtutor/raycasting.html
  (single page, read in full: 933 extracted lines,
  introduction through the textured raycaster)

## Honest scope statement — read this first

**This builds a raycaster, NOT a true 3D renderer.**
The tutorial says so itself: raycasting is a "fast semi-3D
technique", while raytracing is "a realistic rendering
technique that supports reflections and shadows in true 3D
scenes". Do not upgrade this project's claims: there are no
spheres, triangles, meshes, Phong shading, shadows, reflections,
or refraction here. What it builds is a real-time, first-person
view of a flat 2D grid world, drawn as vertical wall stripes
whose height depends on distance.

## What it will be

- A 2D grid world (24 x 24 in the tutorial), where each cell
  is empty space or a wall
- One ray per vertical screen stripe (640 stripes at 640x480),
  stepped through the grid with DDA (Digital Differential
  Analysis) until it hits a wall
- Wall height from the *perpendicular* wall distance, so a
  wall seen straight on is flat, not rounded by fisheye
- Player movement and rotation, frame-time independent,
  with point collision (you cannot walk into a wall)
- An FPS counter
- Then a textured version: a 2D screen buffer, 64x64
  procedural textures, one texture per wall number, with
  y-side walls darkened for a hint of depth

## Planned run (not working yet — code is not written)

The tutorial's output is a real-time 640x480 screen, not an
image file. No image-file output is stated in the tutorial.
The exact display backend (how the screen buffer reaches a
window on a given machine) is **not** specified in these docs
and must be chosen and recorded before code — see SPEC
section 6 and TICKETS T1. Do not assume a backend silently.

## Language

C-like / C++ pseudocode, following the tutorial. The tutorial
notes a portability point that these docs keep: C++, Java and
JavaScript give IEEE-754 infinity for division by zero, which
the DDA code relies on in places, while Python does not — so
a Python port would need the tutorial's explicit `1e30`
guard instead. Language choice for the actual build is C++
(the tutorial's language); the code is written from the
tutorial's explanation, not copied verbatim from it.

## Docs (same structure as distributed-systems)

- `docs/SPEC.md` — the algorithm, the world, the camera,
  and every deliberate divergence and non-goal
- `docs/DESIGN.md` — data structures, the DDA loop, the
  render loop, and the display-backend open question
- `docs/TEST_PLAN.md` — tests based on observable behaviour
  only (wall height, fisheye, textures, movement/collision)
- `docs/TICKETS.md` — build order, following the tutorial's
  sections, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Toolchain note (honest)

A C++ toolchain in this VM is **not verified**. Before any
code is written, a pre-code step must confirm a working C++
compiler and a display backend on the machine that will run
the program. Until then, no "tested" claim is made for this
project. See TEST_PLAN, "Pre-code toolchain check".

## Out of scope, honestly

Stairs, jumping, and height differences (the tutorial's
Wolfenstein world has equal-height, orthogonal walls only);
floors and ceilings; sprites (enemies, objects, goodies —
the tutorial says sprites "aren't discussed in this tutorial
for now"); sloped walls, different wall heights, transparent
walls, and the more advanced Doom/Duke Nukem 3D-style engines
the tutorial mentions but does not build. Those are not
claimed.

## Next step

Instruction for this project comes from the parent agent:
docs first (this set), then — only on approval — code,
test, review, and push, as its own folder under
`3d-renderer/`, alongside the other 10 links' folders.
