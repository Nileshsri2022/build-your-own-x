# DESIGN — A 3D Modeller

## Layout

Python, one small package. File names follow the chapter's
class names, which is what matters for readability; the
chapter prints excerpts, not a full file listing, so this
layout is our organisation of its design, not a claim
about the chapter's files.

```
modeller/
  __main__.py        # entry point: build Viewer, enter main loop
  viewer.py          # Viewer: window, OpenGL setup, render pass, main loop
  scene.py           # Scene: node list, selected node, place depth, render
  nodes.py           # Node, Primitive, Cube, Sphere, HierarchicalNode, SnowFigure
  interaction.py     # Interaction: callback table, OS callbacks, trackball wiring
  trackball.py       # thin wrapper around the Glumpy trackball (imported, not reimplemented)
tests/
  test_scene.py      # scene graph, add/select/render-order (headless)
  test_nodes.py      # transforms, AABB, colour cycling, scaling (headless)
  test_picking.py    # pick-ray construction, AABB hit, closest-hit (headless)
  test_interaction.py# callback register/trigger, no-selection safety (headless)
  manual_checklist.md# window-dependent checks (see TEST_PLAN)
```

Dependencies: Python, numpy (matrices, vectors, inverse
ModelView), PyOpenGL-style OpenGL bindings (`gl*` calls),
GLUT (window, display function, mouse/keyboard callbacks,
main loop), and the Glumpy trackball (quaternions). Their
presence here is **not verified** — see TEST_PLAN T0.

## Viewer

One `Viewer` instance, created once, owns everything:

- `init_interface`: create the 640 x 480 window titled
  "3D Modeller", single-buffered RGB mode, register the
  render function as the display callback.
- `init_opengl`: enable backface culling (cull back),
  depth test (less-than), one light (position (0, 0, 1, 0),
  spot direction (0, 0, -1)), colour material (front and
  back, ambient and diffuse), clear colour (0.4, 0.4,
  0.4, 0.0).
- `init_scene`: build the `Scene`, then the sample scene
  (cube at (2, 0, 2), sphere at (-2, 0, 2), snow figure
  at (-2, 0, -2), each with its colour index).
- `init_interaction`: build the `Interaction` object and
  register its application callbacks (pick, move, place,
  rotate_colour, scale).
- `main_loop`: hand control to GLUT's main loop. It never
  returns; all further work happens in callbacks.
- `render`: initialise the projection (perspective, 70
  degrees, window aspect, near 0.1, far 1000.0, camera
  translated to (0, 0, -15)), load the ModelView matrix
  from the interaction state (translation plus trackball
  matrix), store the ModelView and its inverse, enable
  lighting, clear colour and depth buffers, render the
  scene, disable lighting, draw the unit grid, flush.
- `init_view` (projection setup) is kept separate from
  `render` so the projection values are in one place and
  testable as constants, even though the GL calls
  themselves need a window.

The camera is fixed. All apparent camera movement is the
scene moving: interaction translation plus the trackball
matrix are applied to the ModelView, not to a camera
object. Keep it that way — adding a camera class would
be a second design, not this chapter's.

## Scene and nodes

- `Scene`: holds `node_list` (in insertion order),
  `selected_node` (a node or none), and `PLACE_DEPTH`
  (15.0). Operations: `add_node` (append), `render`
  (render each node in order). Rendering order is
  insertion order; depth testing, not painter's order,
  resolves overlap.
- `Node` (base): holds a translation matrix and a
  scaling matrix (separate, not fused, so scaling a node
  does not move it and translating it does not rescale
  it), a colour index, and an axis-aligned bounding box
  (AABB). Operations: `translate`, `scale` (multiply
  the scaling matrix by 1.1 or 0.9), `rotate_colour`
  (advance the colour index), `render` (apply its
  matrices, then render its content). The AABB is
  computed from the node's geometry in its local space
  and transformed for picking — see picking below.
- `Primitive` (a `Node`): renders through an OpenGL
  call list. The call list is built once per primitive
  type (the chapter does not print these definitions;
  the code must build them, and the test plan covers
  them only through their visible effect, not by
  re-printing chapter code).
- `Cube` and `Sphere`: the only two primitives. A cube
  is one call list; a sphere is one call list. No
  parameters for size at construction in the chapter's
  design — size comes from scaling the node.
- `HierarchicalNode` (a `Node`, the Composite pattern):
  holds child nodes. Its render pushes the ModelView
  matrix, applies its own translation/scaling, renders
  each child in order, then pops. Children therefore
  inherit the parent's transform and can be nested
  arbitrarily deep without the scene code knowing
  anything about their internals.
- `SnowFigure` (a `HierarchicalNode`): three spheres
  grouped as one node. To the scene it is one node: it
  is selected, moved, scaled, and coloured as a unit
  (its children keep their relative positions).

Traversal rule, stated once and used everywhere: a
node's render applies its own transform, renders its
own content (or, for a hierarchical node, its children),
and restores the previous matrix. No node leaks its
transform into a sibling.

## Interaction

- `Interaction`: holds a callback table (a mapping from
  application event name to function) and the trackball
  state, plus the current translation. Operations:
  `register_callback(name, function)` and
  `trigger(name, ...)` (call the registered function,
  if any). The table's names are exactly: pick, move,
  place, rotate_colour, scale.
- OS callbacks (GLUT mouse/keyboard) are thin: they
  translate a raw event into one application event and
  trigger it. No scene logic lives in an OS callback.
  Keyboard mapping: `s` → place sphere, `c` → place
  cube, up/down → scale, left/right → rotate_colour.
  Mouse: click → pick, drag → move/translate,
  wheel → Z translation, trackball drag → rotation.
- Trackball: imported from Glumpy, driven by
  quaternions. The interaction stores the trackball's
  matrix and multiplies it into the ModelView. Do not
  reimplement the trackball here — the chapter imports
  it, and reimplementing quaternion trackball math
  would be a second project hiding inside this one.
  The wrapper exists only so the rest of the code does
  not import Glumpy directly in several places.
- Moving a node: take the mouse ray at the node's
  stored depth, convert it from camera space to world
  space with the inverse ModelView matrix, and set the
  node's translation to that position. Placing a node:
  same conversion, at PLACE_DEPTH, then add the new
  node to the scene and (by the chapter's design)
  make it the new node in the scene.

## Picking (the only ray in this project)

- Build a ray from the mouse position: unproject the
  screen point at two depths, 0.001 and 0.999, giving a
  near point and a far point; the ray runs between them.
- Test the ray against every node's AABB. Keep the
  closest hit (smallest distance along the ray).
  Select that node; if nothing is hit, select nothing.
- Known limitation, by design: AABB picking is loose
  for spheres. A ray through the corner region of a
  sphere's box selects the sphere even though it misses
  the sphere itself. This is documented in SPEC and
  tested in TEST_PLAN; it is not a bug to be fixed
  silently by switching to exact sphere intersection,
  because that would change the chapter's picking
  model (accurate intersection is a listed future
  extension, not this build).
- Picking runs on AABBs in world space: a node's AABB
  must be transformed by the node's full transform
  (including its parents', for hierarchical nodes)
  before testing. A picking test that ignores parent
  transforms will pass for top-level nodes and fail
  for figures — test it explicitly.

## Numbers and determinism

- All matrices and vectors are numpy arrays. The
  ModelView matrix and its inverse are stored at render
  time and reused by picking and moving, so picking
  and moving always agree with what was last drawn.
- Scale factors are exactly 1.1 (up) and 0.9 (down),
  applied multiplicatively, so repeated scaling
  compounds (1.1^n, 0.9^n) rather than adding.
- Colour cycling wraps: after the last colour in the
  list comes the first again. The colour list itself is
  a code decision (the chapter cycles a list; its exact
  entries are not a spec requirement), but the wrap
  behaviour is.
- Place depth is exactly 15.0. Pick-ray depths are
  exactly 0.001 and 0.999. Projection values are exactly
  those in SPEC section 3. Tests assert these constants.

## Failure modes

- No dependency (numpy / OpenGL / GLUT / Glumpy):
  fail at startup with a message naming the missing
  piece.
- No selection: scale, rotate_colour, and move do
  nothing, without crashing.
- Empty scene: render draws only the grid; picking
  selects nothing; placing adds the first node.
- A pick ray hitting nothing: selection becomes none.
- A window close: the program ends; there is no
  save-on-close, because there is no save feature.
