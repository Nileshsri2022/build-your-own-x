# GLOSSARY — A 3D Modeller

- **Modeller.** A program for designing 3D objects
  interactively, as opposed to a renderer, which
  produces an image from a scene. This project is a
  modeller.
- **CAD.** Computer-aided design: software that helps
  design physical or virtual objects before building
  them, by representing them on a 2D screen.
- **Scene graph.** A tree of nodes representing a
  design. Traversing it renders the design; grouping
  nodes lets a whole assembly be treated as one node.
- **Node.** One element of the scene graph, holding a
  translation matrix, a scaling matrix, a colour,
  and a bounding box.
- **Primitive.** A node rendered from a fixed
  geometric definition via an OpenGL call list.
  Here, the only primitives are cube and sphere.
- **Hierarchical node.** A node that groups child
  nodes (the Composite pattern). Rendering it renders
  its children under its own transform.
- **Snow figure.** The chapter's example hierarchical
  node: three spheres grouped as one figure.
- **Call list.** An OpenGL list of drawing commands,
  recorded once and replayed on every render, used
  here for primitives.
- **Model space.** The coordinate space a mesh is
  defined in, centred on itself.
- **World space.** The coordinate space of the whole
  scene, after model matrices are applied.
- **Eye space.** The coordinate space as seen from
  the camera, after the view matrix is applied.
- **ModelView matrix.** The combined model and view
  matrix: model space → world space → eye space.
  This project stores it and its inverse at render
  time for picking and moving.
- **AABB.** Axis-aligned bounding box: the smallest
  box, aligned with the coordinate axes, that
  encloses an object. Used here for picking; it
  encloses a sphere loosely, so corner-region hits
  are a known approximation.
- **Picking.** Selecting an object by clicking on
  it: a ray is built from the mouse position and
  tested against objects (here, their AABBs).
- **Pick ray.** The ray from the mouse into the
  scene, built here by unprojecting the screen
  point at depths 0.001 and 0.999.
- **Trackball.** A virtual sphere used to rotate a
  scene by dragging: the drag maps onto rotation
  of the sphere. Implemented with quaternions,
  which avoid gimbal lock.
- **Gimbal lock.** Loss of a rotation degree of
  freedom when two rotation axes align, causing a
  jump or flip. Quaternion trackballs avoid it.
- **Fixed-function pipeline.** Legacy OpenGL's
  built-in pipeline: lighting, colouring, and
  culling controlled by global state, rather than
  by user-written shaders.
- **GLUT.** The OpenGL Utility Toolkit: creates
  windows and registers callbacks for mouse,
  keyboard, and display events.
- **Place depth.** The depth (15.0) at which new
  nodes are placed along the pick ray.
- **Emission.** A material property that makes an
  object appear to glow. Used here to mark the
  selected node.
- **Depth test.** Comparing each new pixel's depth
  against the stored depth, keeping only the
  nearer surface (here, the less-than function).
- **Backface culling.** Discarding polygons that
  face away from the viewer, so only front faces
  are drawn.
- **Composite pattern.** A design pattern in which
  a group of objects is treated as a single object.
  Here, a hierarchical node and its children are
  one node to the scene.
