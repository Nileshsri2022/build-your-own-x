# SPEC — A 3D Modeller (AOSABOOK, 500 Lines or Less)

## 1. Idea

A CAD tool, at its core, must do three things: hold a data
structure for the design being built, display that design on
a 2D screen, and let the user add to and modify it. This
project builds exactly those three things, at small scale,
in Python. It is a modeller, not a renderer: display is
handed to OpenGL, and the project's own work is the scene
graph, the viewer, and the interaction.

The chapter's driving principle is that the rendering
process shapes the data structure: a tree of nodes that can
be traversed for rendering, with new node types addable
without added traversal complexity, keeps the rendering
logic simple even as designs get complex.

## 2. The chapter, section by section

- **Introduction.** CAD tools abstract a 3D design for a 2D
  screen. They need: a data structure for the design, a way
  to display it, and a way to interact with it. Saving and
  loading designs is also named as a general CAD need — but
  it is **not** built in this chapter (see section 6).
- **Rendering as a Guide — Managing Interfaces and the
  Main Loop.** Before rendering: create a window, talk to
  graphics drivers through an abstraction (OpenGL), and use
  GLUT to manage the window and callbacks. Then a main loop
  drives rendering and events.
- **A Note About OpenGL.** OpenGL is a cross-platform
  graphics API. It has two variants: Legacy OpenGL (fixed-
  function pipeline — lighting, colouring, face culling
  enabled by global state) and Modern OpenGL (programmable
  pipeline — user-written shaders on the GPU). This project
  uses Legacy OpenGL despite it being deprecated, because
  its fixed functionality keeps the code small, reduces
  the linear-algebra knowledge required, and simplifies
  the code. Rendering is based on polygons defined by
  vertices and normals; to render one side of a cube, four
  vertices and a normal are specified.
- **About GLUT.** The OpenGL Utility Toolkit, bundled with
  OpenGL, creates operating-system windows and registers
  user-interaction callbacks. That basic functionality is
  sufficient here; a fuller toolkit (GTK, Qt) is named only
  as an alternative for a more full-featured tool.
- **The Viewer.** One `Viewer` instance manages window
  creation and rendering and contains the main loop. Its
  initialisation, in order: create the window and register
  the render function; set up OpenGL state; create the
  scene and place initial nodes; register interaction
  callbacks. Then the GLUT main loop takes over and never
  returns; registered callbacks fire on events.
- **Coordinate Space.** A coordinate space is an origin plus
  a set of basis vectors (usually x, y, z axes). A point is
  an offset in x, y, z from the origin of its space; a
  vector is the difference between two points. A
  transformation matrix converts a vector from one space
  to another (translation, scaling, rotation, and so on).
  Spaces in play: model space (a mesh's own space), world
  space, eye space, and the spaces OpenGL handles from eye
  space onward (clip, normalized device, viewport). This
  project manages the left-hand side itself: a model
  matrix (model → world) and a view matrix (world → eye),
  combined into a ModelView matrix. The right-hand side
  (eye → clip via perspective, clip → device/viewport)
  is handled by OpenGL.
- **Rendering with the Viewer.** The render pass: set up
  the projection, initialise the ModelView matrix from
  the interaction state, clear the screen, tell the scene
  to render itself, render a unit grid, and flush the
  buffer for display. Lighting is enabled for the scene
  and disabled for the grid, so the grid renders in solid
  colours and stays visually distinct from the scene.
- **What to Render: The Scene.** The scene holds a list of
  nodes, a selected node, and a place depth. Its render
  operation renders each node in turn. The sample scene
  contains a cube, a sphere, and a hierarchical snow
  figure (see section 4).
- **Nodes.** See DESIGN for the class structure. In short:
  a base node with translation and scaling matrices, a
  colour, and an axis-aligned bounding box (AABB); primitive
  nodes rendered via OpenGL call lists; cube and sphere
  primitives; and hierarchical nodes (the Composite
  pattern) that group child nodes, rendered by traversing
  the tree with ModelView push/pop.
- **User Interaction — the Interaction class.** A small
  callback system for application-level events: the class
  holds a table mapping application event names to
  functions, with register and trigger operations. This
  encapsulates operating-system events behind application
  events, so the scene logic never talks to GLUT directly.
  Application events: pick, move, place, rotate_colour,
  scale.
- **Operating System Callbacks.** GLUT mouse and keyboard
  callbacks translate raw events into application events.
  Keys: `s` places a sphere, `c` places a cube, up/down
  arrows scale, left/right arrows rotate colour. Mouse
  actions (drag, wheel, click) drive move, translate,
  trackball rotation, and picking (see section 5).
- **Moving, rotating, translating the scene.** The camera
  is fixed; the scene moves instead. Rotation uses a
  trackball (imported from Glumpy), driven by quaternions,
  which avoids gimbal lock. Translation is by mouse drag,
  with the wheel moving along Z. A stored inverse
  ModelView matrix converts between spaces where needed.
- **Selecting scene objects.** Picking builds a ray from
  the mouse position into the scene (unprojecting at two
  depths), tests it against each node's AABB, and selects
  the closest hit. This is an approximation: an AABB
  encloses a sphere loosely, so a ray can hit the box
  corner region without hitting the sphere itself. That
  error is documented, not hidden — see section 6.
- **Modifying scene objects.** Changing colour cycles
  through a colour list. Scaling multiplies by 1.1 or 0.9.
  Moving a node uses a ray at the node's stored depth,
  converted to world space with the inverse ModelView
  matrix. Placing a node creates it at the place depth
  along the pick ray, converting the camera-space
  position to world space the same way; placing also
  accepts a 'figure' (snow figure) as well as cube and
  sphere.
- **Summary and Further Exploration.** The chapter's own
  summary: an extensible scene-graph data structure, OpenGL
  matrix manipulation during traversal, a simple callback
  system, and scene manipulation. Its suggested extensions
  are listed in section 6 as future work only.

## 3. Viewer and camera (exact values from the chapter)

- Window: 640 x 480, single-buffered RGB display mode,
  title "3D Modeller".
- Projection: perspective with a 70-degree field of view,
  aspect ratio from the window size, near plane 0.1, far
  plane 1000.0, and the camera translated to (0, 0, -15)
  (i.e. looking at the scene from 15 units back).
- OpenGL state at setup: backface culling enabled, culling
  the back face; depth testing enabled with the less-than
  function; one light enabled; colour material enabled
  for front and back, ambient and diffuse; clear colour
  a mid-grey (0.4, 0.4, 0.4, 0.0).
- Grid: drawn after the scene, with lighting disabled.

## 4. Scene contents

- Sample scene (built at startup): a cube translated to
  (2, 0, 2) with one colour index, a sphere translated to
  (-2, 0, 2) with another colour index, and a hierarchical
  snow figure translated to (-2, 0, -2).
- Snow figure: three spheres grouped as one hierarchical
  node (the Composite pattern in action — the figure is
  treated as a single node by the scene, and its children
  are rendered by traversal).
- Place depth: 15.0. New nodes are placed at this depth
  along the pick ray.
- Selected node: exactly one node is selected at a time
  (or none before the first pick); it glows via emission
  so selection is visible.

## 5. Interaction contract (what every input must do)

- Click (pick): build a pick ray, test AABBs, select the
  closest hit; a miss selects nothing (previous selection
  behaviour must be defined in code and tested — see
  TEST_PLAN).
- Drag (move/translate): move the scene (camera is fixed)
  in the drag plane; the wheel moves along Z.
- Trackball drag (rotate): rotate the scene via the
  trackball's quaternion; no gimbal-lock jump when
  passing through poles.
- Key `s`: place a sphere. Key `c`: place a cube.
  Placing a figure is also supported by the place
  operation (the chapter's place handler accepts it).
- Up/down arrows: scale the selected node by 1.1 / 0.9.
- Left/right arrows: cycle the selected node's colour.
- Moving a selected node: the node follows the mouse at
  its stored depth, converted through the inverse
  ModelView matrix.
- All of the above with no selected node must be safe:
  scale, rotate-colour, and move must do nothing (not
  crash) when nothing is selected.

## 6. Lighting, and what is NOT here

- Lighting is legacy OpenGL fixed-function only: one
  light, positioned at (0, 0, 1, 0) (a directional light,
  since the fourth component is 0), with a spot direction
  of (0, 0, -1); colour material for ambient and diffuse;
  backface culling; depth test. The selected node glows
  through emission. There is no hand-written Phong
  shader, no custom shader of any kind, no shadows, no
  reflections, no refraction, no textures, and no
  anti-aliasing in this project's own code.
- Not built here (chapter's future extensions only, not
  to be claimed): triangle-mesh nodes, an undo stack,
  save/load (e.g. via DXF), export to a photorealistic
  renderer, and accurate ray-object intersection.
  Triangle meshes are explicitly outside this project's
  scope; the only primitives are cube and sphere (plus
  their grouping into figures).
- Picking is a bounding-box approximation. A ray that
  passes through a sphere's AABB corner region, but not
  through the sphere, will still select the sphere. That
  is a documented limitation of the design, and tests
  must cover it (TEST_PLAN) rather than pretend picking
  is exact.

## 7. Output and failure modes

- Output is the interactive window only. Closing the
  window ends the program. There is no file output, no
  headless render, and no batch mode in scope.
- A missing dependency (numpy, OpenGL bindings, GLUT,
  Glumpy trackball) at startup must fail with a clear
  message naming the missing piece, not a bare traceback
  buried in viewer setup — the exact wording is a code
  decision, the requirement is not.
- Interaction with no selection, an empty scene, and a
  pick ray that hits nothing are normal states, not
  errors, and must not crash.
