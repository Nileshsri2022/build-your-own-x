# A 3D Modeller — Build Your Own 3D Renderer (AOSABOOK link)

Status: **docs only, no code, no push.**
This folder is one of the eleven separate docs sets under
`3d-renderer/`, one per BYOX 3D Renderer link. It covers only
this link; it does not change the other ten folders.

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _A 3D Modeller_, in _500 Lines or Less_
  (Architecture of Open Source Applications) —
  http://aosabook.org/en/500L/a-3d-modeller.html
- Language in the tutorial: Python, with numpy, PyOpenGL-style
  OpenGL calls, and GLUT for the window and callbacks.
- Output: an interactive OpenGL/GLUT window, 640 x 480, titled
  "3D Modeller". There is no image-file output (no PNG, PPM,
  or TGA) in this build.

## The honest headline — read this first

This builds a **modeller / interactive CAD-style editor that
USES OpenGL**, not a rendering algorithm. Rendering is delegated
to legacy OpenGL's fixed-function pipeline. This project contains
no rasterization code of its own and no ray-tracing code of its
own. The only ray use in its own code is picking: a ray built
from the mouse position to test which object's bounding box it
hits. Do not describe this as a raytracer, a rasterizer, or a
renderer built from scratch — the chapter itself calls it
"a tiny 3D modeller", and these docs follow that claim exactly.

What it does teach, and what it does build, is the rest of a
3D editor: a scene-graph data structure for the design, a
viewer that displays it, and mouse/keyboard interaction for
editing it (pick, move, place, scale, rotate colour, trackball
orbit). That is the chapter's real substance.

## What the user will be able to do

- Open a 640 x 480 window showing a sample scene: a cube, a
  sphere, and a snow figure (three spheres grouped together).
- Orbit the scene with a trackball, translate it by dragging,
  and zoom along Z with the mouse wheel.
- Click to select an object (closest bounding-box hit wins).
- Move the selected object, scale it up/down, and cycle its
  colour.
- Place a new cube (key `c`), a new sphere (key `s`), or a new
  snow figure, at a fixed depth along the pick ray.
- See the selected object glow, so selection is visible.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — the chapter, section by section, and every
  deliberate scope boundary
- `docs/DESIGN.md` — classes, scene graph, viewer loop,
  interaction callbacks
- `docs/TEST_PLAN.md` — behaviour tests, split into
  headless-testable parts and window-dependent parts
- `docs/TICKETS.md` — build order, following the chapter's
  sections, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Divergences and decisions, stated up front

- None in scope. The chapter's scope is followed as-is:
  legacy OpenGL fixed-function rendering, GLUT window and
  callbacks, numpy matrices, and a trackball imported from
  Glumpy (not reimplemented here, following the chapter).
  Modern OpenGL / shaders are deliberately not substituted —
  the chapter chose legacy OpenGL precisely to keep the code
  small and the linear-algebra requirements low, and swapping
  it would turn this into a different project.
- The "500 lines" figure is the chapter's framing (the book
  series is called "500 Lines or Less"). It is **not** an
  independently verified count: the chapter prints substantial
  excerpts but omits some parts (primitive call-list
  definitions are described as not shown, and the trackball
  is imported rather than printed). Do not quote 500 lines
  as a measured result of this build until the code is
  written and counted.

## Out of scope, honestly

Save/load of designs (stated in the chapter as a general CAD
requirement, not built here; a DXF save/load is listed only
as a future extension), triangle-mesh nodes (explicitly
"outside the scope of this project"), undo/redo, export to a
photorealistic renderer, accurate ray-object intersection
(the picking here is a bounding-box approximation, with a
known error at sphere corners), Phong shading implemented
by hand, custom shaders, shadows, reflections, refraction,
textures, and anti-aliasing. None of those are claimed; the
lighting here is exactly one fixed-function OpenGL light
(see SPEC).

## Dependencies — not yet verified here

Python, numpy, PyOpenGL-style OpenGL bindings, GLUT, and the
Glumpy trackball are required to run this. **Their presence
in this VM has not been verified.** Checking them is a
pre-code step in TEST_PLAN (T0), before any build claim is
made. Do not claim this runs headlessly in this VM until
that check passes — the window itself cannot run headlessly
in any case; only the scene-graph, transform, and picking
parts can be tested without a window.

## Next step

User approval, then code (TDD where headless testing is
possible, ticket by ticket), then a separate per-push
approval before anything is pushed. Code is not written yet.
