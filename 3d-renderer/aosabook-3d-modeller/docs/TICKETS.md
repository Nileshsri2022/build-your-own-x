# TICKETS — A 3D Modeller

Build order follows the chapter's sections. Each ticket
is done (its tests passing, or, for window work, its
manual checks run) before the next starts, except
where noted. No ticket starts work for a later section
early, and no ticket adds a future-extension feature.

- **T0 — Dependency check.** Check Python, numpy,
  OpenGL bindings, GLUT, and the Glumpy trackball on
  the build machine; record the result. Blocks: all
  code tickets (a missing dependency changes what can
  be tested, and must be known before coding, not
  discovered at the end). Tests: none — this ticket's
  output is the recorded check itself.
- **T1 — Coordinate spaces and matrices.** Numpy
  helpers for points, vectors, and transformation
  matrices; model / world / eye space conversions;
  ModelView and its inverse. Blocks: everything.
  Tests: transform tests (separate matrices, inverse
  round-trip, projection constants).
- **T2 — Scene and nodes.** `Scene`, `Node`,
  `Primitive`, `Cube`, `Sphere`, `HierarchicalNode`,
  `SnowFigure`, and the sample scene. Rendering here
  is through a stub matrix stack, not a real window,
  so the graph logic is testable headlessly.
  Blocks: T3–T6. Tests: scene-graph tests.
- **T3 — Viewer and window.** `Viewer` initialisation
  (interface, OpenGL state, scene, interaction),
  the render pass (projection, ModelView, scene,
  grid, flush), and the GLUT main loop. Blocks: T4
  (interaction callbacks need a viewer to attach
  to). Tests: manual checklist items for window,
  scene, and grid only.
- **T4 — Interaction callbacks.** `Interaction`
  callback table (pick, move, place, rotate_colour,
  scale), OS callbacks, and the keyboard mapping
  (`s`, `c`, arrows). Blocks: T5–T6. Tests:
  callback tests, no-selection safety, placement.
- **T5 — Trackball, translation, and moving.**
  Glumpy trackball wiring, drag translation, wheel
  Z movement, and moving a selected node at its
  stored depth via the inverse ModelView. Blocks:
  T6. Tests: transform tests, move tests, manual
  orbit/translation checks (including the
  gimbal-lock check).
- **T6 — Picking and modifying.** Pick-ray
  construction (unproject at 0.001 / 0.999), AABB
  testing, closest-hit selection, colour cycling,
  and scaling (1.1 / 0.9). Blocks: T7. Tests:
  picking tests, including the documented
  AABB-corner case, and the modifying tests.
- **T7 — Code review + push prep.** Review against
  SPEC, run all headless tests, run the manual
  checklist (or list its gaps), count the code
  lines before any "500 lines" claim, and present
  for per-push approval. Push itself is separate,
  on approval.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6 → T7, strictly
linear. The chapter's order is a dependency chain
(data structure, then display, then interaction,
then picking on top of interaction), and a parallel
build would hide a scene-graph bug behind a
window bug. There is no parallel ticket here.
