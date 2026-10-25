# TEST_PLAN — A 3D Modeller

## T0 — Dependency check (pre-code step, do this first)

Before any build claim is made, check, on the machine
where the code will run, whether Python, numpy,
PyOpenGL-style OpenGL bindings, GLUT, and the Glumpy
trackball are importable and usable. **None of these is
verified in this VM at docs time — do not claim they
are.** Record the result (present / absent, with
versions) in the code review (T7). If GLUT or the
trackball is absent, the window cannot be run here;
that does not block the headless tests below, but it
does block any "window verified" claim, and the gap
must be reported as such.

## Headless tests (no window needed — run these in CI)

These use plain Python test functions (pytest-style, or
the standard library's unittest — a code decision, not
a spec requirement). They construct scene objects
directly, with no OpenGL context, so any OpenGL call
in the code under test must be isolated behind the
viewer/render path, not inside scene/node logic.
If a headless test needs a window, that is a design
bug, not a test problem.

- **Scene graph.**
  - Adding nodes appends them in insertion order;
    rendering order follows insertion order.
  - A scene starts with no selected node; selecting a
    node sets it; selecting nothing clears it.
  - The sample scene builds exactly three top-level
    nodes (cube, sphere, snow figure), with the
    translations in SPEC section 4.
  - A hierarchical node renders its children and
    restores the matrix afterwards: a sibling rendered
    after a figure is not transformed by the figure
    (test by recording the transform stack, with a
    stub matrix stack standing in for OpenGL).
  - A snow figure is one node to the scene: selecting,
    moving, scaling, or colouring it affects the
    figure as a unit, and its children keep their
    relative positions.
- **Transforms.**
  - Translation moves a node without changing its
    scale; scaling changes its scale without moving
    it (separate matrices — test both directions).
  - Scaling by 1.1 then 0.9 does not return exactly
    to the original (1.1 x 0.9 = 0.99) — test the
    multiplicative behaviour, not an assumed round-trip.
  - Colour cycling wraps from the last colour back
    to the first.
  - ModelView and inverse ModelView are inverses:
    converting a point to world space and back
    returns the original within floating-point
    tolerance (1e-9 relative).
  - Projection constants are exactly those in SPEC
    section 3 (70 degrees, near 0.1, far 1000.0,
    camera at (0, 0, -15), place depth 15.0, pick
    depths 0.001 / 0.999).
- **Picking.**
  - A ray through the centre of a top-level cube's
    box selects that cube.
  - A ray through two overlapping boxes selects the
    closer one.
  - A ray hitting nothing selects nothing.
  - A ray through the corner region of a sphere's
    AABB, missing the sphere itself, still selects
    the sphere — this is the documented AABB
    approximation, and the test asserts it, so a
    future "fix" to exact sphere picking would
    fail this test and have to be a deliberate,
    documented change.
  - Picking a child of a hierarchical node, after
    the parent's transform is applied, selects the
    figure (not nothing, not a sibling) — parent
    transforms must not be ignored.
- **Interaction callbacks.**
  - Registering a callback and triggering its name
    calls it; triggering an unregistered name does
    nothing (no crash).
  - With no selection, scale, rotate_colour, and
    move do nothing and do not crash.
  - Placing a sphere, a cube, and a figure each add
    exactly one top-level node, at the place depth
    along the pick ray, in world space.
  - Moving a selected node to a given mouse ray
    sets its translation to the converted world
    position at its stored depth.

## Window-dependent checks (manual checklist, not CI)

These require a real window, a real OpenGL context,
and a human (or a screen-capture harness) looking at
it. They are recorded in `tests/manual_checklist.md`
and checked off by hand at review time. They are not
claimed as passing until actually run and seen.

- [ ] Window opens at 640 x 480, titled "3D Modeller",
  showing the sample scene (cube, sphere, snow figure)
  plus a unit grid, on a mid-grey background.
- [ ] Dragging orbits the scene smoothly via the
  trackball, with no jump or flip when passing over
  the top (gimbal-lock check).
- [ ] Mouse-wheel moves the scene along Z; drag
  translates it in the plane.
- [ ] Clicking an object selects it and it visibly
  glows; clicking empty space clears selection.
- [ ] With an object selected, up/down arrows scale
  it visibly (larger / smaller), left/right arrows
  cycle its colour.
- [ ] Pressing `s` adds a sphere, `c` adds a cube,
  and the new object appears at the expected depth
  and can then be selected and moved.
- [ ] Closing the window ends the program cleanly.

## What must NOT be claimed

- No "500 lines" claim until the code is written and
  counted (see README).
- No "runs in this VM" claim until T0 passes and, for
  the window checks, a window is actually opened.
- No save/load, triangle meshes, undo, shadows,
  reflections, textures, or exact picking — none of
  those are in this build, and a test must not be
  written for a feature that does not exist.
- No rendering-algorithm claim: this project does
  not implement rasterization or ray tracing, and
  its tests must not describe it as doing so.

## Before any push

- T0 result recorded.
- All headless tests pass.
- Manual checklist either fully checked off, or its
  unchecked items listed as known gaps in the review.
- Code review against SPEC: every section-2 behaviour
  present, every section-6 exclusion still excluded,
  no copied chapter code (implementation is ours,
  following the chapter's design).
