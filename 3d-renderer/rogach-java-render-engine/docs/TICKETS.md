# TICKETS — Rogach Pure Java 3D Render Engine

Build order follows the article's sections. Each
ticket is done (code + its tests passing) before the
next starts, except where noted. No ticket starts
code for a later section early.

- **T1 — Window wrapper + geometry (article: GUI,
  vertices/triangles, tetrahedron).** `Vertex`,
  `Triangle`, tetrahedron builder (4 triangles,
  white/red/green/blue, centred at origin),
  `DemoViewer` shell with heading/pitch sliders
  that repaint (rendering itself comes in T3–T5;
  for T1 the panel may draw a blank image, and the
  test is that the window opens and sliders move
  without error — manual check, recorded as such).
  Blocks: everything. Tests: geometry construction
  (4 triangles, colours, centred at origin).
- **T2 — Rotation (article: rotation matrices).**
  `Matrix3` (multiply, transform), heading and
  pitch rotation builders, single-matrix composition.
  Blocks: T3–T6. Tests: Matrix3Test (identity,
  90-degree rotations, composition equals sequential
  application).
- **T3 — Wireframe (article: orthographic
  projection).** Orthographic projection (discard z
  after transform + screen-centre translation) and
  triangle-outline drawing, as a stepping stone:
  the renderer at this stage draws outlines only,
  so the tetrahedron's shape is checkable before
  filling exists. Blocks: T4. Tests: a rendered
  wireframe image has coloured outline pixels and
  an uncoloured interior (the inverse of the T4
  test, so a T3/T4 mix-up is caught).
- **T4 — Rasterization + z-buffer (article:
  barycentric rasterization, z-buffer).**
  Bounding-box pixel loop, barycentric inside test,
  direct `BufferedImage` writes, per-pixel depth
  array with barycentric-interpolated depth.
  Blocks: T5. Tests: RasterTest (coverage, edge
  rule, clamping, degenerate triangle) and the
  depth-ordering test.
- **T5 — Flat shading (article: flat shading).**
  `Shade` (normal via cross product, |norm.z|
  coefficient, sRGB 2.4 round-trip), applied once
  per triangle before its pixel loop. Blocks: T6.
  Tests: ShadeTest (facing, sideways, 45 degrees,
  shading follows rotation, 0/1 endpoints).
- **T6 — Inflate bonus + PNG output (article:
  bonus; PNG is ours).** `Inflate` (subdivide ×4
  + renormalise to sphere radius), `PngWriter`,
  `Main` argument handling (optional PNG path/size,
  bad-argument errors). Blocks: T7. Tests:
  InflateTest (counts, on-sphere vertices, colour
  inheritance) and the integration script.
- **T7 — Code review + push prep.** Review against
  SPEC, check no out-of-scope claims, run the full
  suite + render tetrahedron and inflated-sphere
  PNGs for eyeballing, do the one manual window
  check (both sliders move the object), then
  present for per-push approval. Push itself is
  separate, on approval, for this folder only.

## Dependencies between tickets

T1 → T2 → T3 → T4 → T5 → T6 → T7, strictly linear.
Each section's behaviour is the foundation for the
next, and a parallel build would hide a regression in
an earlier section behind a later one. T1's window
is the one exception that is finished in T7's manual
check, not by an automated test.
