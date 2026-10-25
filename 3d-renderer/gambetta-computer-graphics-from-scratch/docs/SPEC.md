# SPEC — Computer Graphics from Scratch (Gambetta)

## 1. Idea

One book, two renderers, one shared starting point. The book
starts from almost nothing — a canvas and a single operation,
`PutPixel`, that sets the colour of one pixel — and builds up
to two complete software renderers that render the same simple
scene by different routes. The raytracer works image-first:
one ray per pixel, into the scene. The rasterizer works
object-first: project triangles onto the screen, then fill
the pixels they cover. This docs set specifies both, as two
sub-builds, and does not merge them into one renderer.

Prerequisites, as the book states them: basic coding, a
little high-school trigonometry at most, and linear algebra
as taught in the book's own appendix (points, vectors,
matrices, in practical form). The book is presented as
understandable by high-school students while rigorous enough
for professional engineers, covering the topics of a full
university course.

## 2. Shared foundation (chapter 1)

- **Canvas.** An abstract drawing surface of a fixed width
  and height. Coordinates are given in canvas space; the
  build must define, once, how canvas coordinates map to
  pixel positions, and both renderers must use that mapping.
- **PutPixel.** The only assumed primitive: set one pixel to
  one colour. Everything else is built on it.
- **Colour.** Colours are represented as RGB triples. The
  exact numeric range (0–255 integers vs. 0–1 floats) is an
  implementation choice for the build and must be fixed in
  DESIGN.md before code; the book's demos use one convention
  and the build must not mix conventions between modules.
- **Linear algebra (appendix).** Points, vectors, and
  matrices, including 4×4 transform matrices for the
  rasterizer. Vector operations needed by both renderers:
  addition, subtraction, scaling, dot product, length, and
  normalisation. Matrix operations needed by Part II:
  multiplication and applying a transform to a point/vector.

## 3. Sub-build A — Part I, Raytracing

### Chapter 2 — Basic Raytracing
- Cast one ray per pixel from the camera into the scene.
- Geometry: spheres only. No triangle meshes in Part I.
- A ray that hits a sphere colours its pixel with that
  sphere's colour; a ray that hits nothing colours its pixel
  with the background colour.
- Result at this stage: spheres that look like flat coloured
  circles (no shading yet).

### Chapter 3 — Light
(per Introduction overview / demo excerpts for the chapter
body beyond the opening sections and summary — verify
before code)
- Three light types: ambient, point, and directional.
- Lights are white only, and there is no distance falloff
  (a point light does not dim with distance).
- Shading model: diffuse reflection plus specular reflection.
  Diffuse strength depends on the angle between the surface
  normal and the light direction; specular strength depends
  on how close the view direction is to the reflected light
  direction, raised to a per-material specular exponent.
  The book's example contrast is a red material with a high
  exponent (500, a tight highlight) against a green material
  with a low exponent (10, a broad highlight) — use these
  values in tests, as they are the book's own example, not
  a claim about the chapter's full text.
- Result at this stage: spheres that look like spheres, not
  circles.

### Chapter 4 — Shadows and Reflections
(per Introduction overview / demo excerpts — verify before
code)
- Shadows: for each light, test whether anything blocks the
  path from the hit point to the light; a blocked light
  contributes no diffuse or specular light for that pixel.
- Reflections: a mirror-like surface reflects its
  surroundings. The reflected colour is obtained by tracing
  a reflected ray recursively, and the final colour blends
  the surface's own (local) colour with the reflected colour
  in proportion to a reflectivity factor `r`:
  `final = local * (1 - r) + reflected * r`.
- Recursion is bounded by a `recursion_depth` limit; beyond
  it, the reflected contribution is the background colour.
- These are raytracer features. They are NOT claimed for the
  rasterizer (see section 4).

### Chapter 5 — Extending the Raytracer
(per Introduction overview — verify before code)
- An overview chapter: additional features that could be
  added to the raytracer, presented as possibilities beyond
  the book's scope, not as features this build implements.
- This build implements nothing from chapter 5 unless it is
  added to this SPEC by a later, explicit decision. In
  particular, refraction and anti-aliasing are NOT in this
  build: neither was seen in the verified material, and
  neither is added here on speculation.

## 4. Sub-build B — Part II, Rasterization

### Chapter 6 — Lines
(per Introduction overview / demo excerpts — verify before
code)
- Draw line segments on the canvas, from a blank canvas up.
  Lines are the first rasterizer primitive and the basis for
  wireframe drawing.

### Chapter 7 — Filled Triangles
(per Introduction overview / demo excerpts — verify before
code)
- Fill triangles with a single colour, reusing the line
  ideas of chapter 6. A triangle is filled by determining
  which pixels it covers and colouring them.

### Chapter 8 — Shaded Triangles
(per Introduction overview / demo excerpts — verify before
code)
- Fill triangles with a smooth colour gradient instead of a
  single colour, by interpolating colour across the
  triangle's pixels.

### Chapter 9 — Perspective Projection
(per Introduction overview / demo excerpts — verify before
code)
- Convert a 3D point into a 2D point that can be drawn on
  the canvas, with perspective: farther objects appear
  smaller. This is the geometry step the rasterizer needs
  before it can render a scene.

### Chapter 10 — Describing and Rendering a Scene
(per Introduction overview / demo excerpts — verify before
code)
- Scene representation: objects are described as a Model
  (vertex and triangle lists — the book's example is a
  cube), an Instance (a Model placed in the scene, with its
  own transform), and a Camera (position and orientation).
  Transforms are 4×4 matrices.
- Rendering a scene means applying the model, instance, and
  camera transforms, projecting, and drawing the resulting
  triangles.

### Chapter 11 — Clipping
(per Introduction overview / demo excerpts — verify before
code)
- Remove the parts of the scene the camera cannot see, at
  both instance level and triangle level, so a scene can be
  rendered safely from any camera position. The book's demo
  case: a cube behind the camera must be correctly
  invisible, not drawn, not crashing the renderer.

### Chapter 12 — Hidden Surface Removal
(per Introduction overview / demo excerpts — verify before
code)
- Combine perspective projection with shaded triangles to
  render solid-looking objects, ensuring distant objects do
  not cover nearer ones. Mechanisms, per demo excerpts:
  depth buffering (a per-pixel depth store) and back-face
  culling (discarding triangles facing away from the
  camera), both toggleable in the book's demos.

### Chapter 13 — Shading
(per Introduction overview / demo excerpts — verify before
code)
- Apply the lighting equation from chapter 3 to whole
  triangles. Three variants, per demo excerpts:
  - Flat shading: one colour per triangle, from the
    triangle's normal.
  - Gouraud shading: colour interpolated across the
    triangle, from per-vertex normals.
  - Phong shading: shading evaluated per pixel (as opposed
    to per vertex), using vertex normals.
- The exact normal source (triangle normal vs. model vertex
  normals) for each variant is a demo-excerpt detail and
  must be verified against the chapter before code; the
  build must not silently pick one convention for all three.

### Chapter 14 — Textures
(per Introduction overview / demo excerpts — verify before
code)
- "Paint" images onto triangles to fake surface detail.
- Interpolation must be perspective-correct: the book's
  demo toggles linear interpolation in screen space against
  perspective-correct interpolation using reciprocal depth
  (1/z), and the difference must be visible in tests. Do
  not ship linear-only texture interpolation and call it
  the book's result.

### Chapter 15 — Extending the Rasterizer
(per Introduction overview — verify before code)
- As with chapter 5, an overview of further features beyond
  the book's scope. Nothing from chapter 15 is implemented
  by this build unless added to this SPEC by a later,
  explicit decision.

## 5. Output and the build's own proposal

- Book output: browser Canvas via `PutPixel`, plus a live
  JavaScript demo per algorithm. No image-file output is
  stated anywhere in the verified material.
- Build proposal (ours, not the book's): JavaScript, run in
  a browser page for the demos, with a test harness that
  renders to an off-screen canvas and reads pixels back
  (see TEST_PLAN.md). This is proposed so renders can be
  checked by pixel values, not only by eye. If a headless
  canvas runner is not available in the build environment,
  that is a toolchain gap to report, not a reason to drop
  pixel tests silently.
- Default scenes (ours, not the book's exact scenes — the
  book's demo scenes were not read in full, so no scene is
  claimed to reproduce them):
  - Raytracer default: three spheres of different colours
    and specular exponents (including the book's 500 vs.
    10 contrast), two lights, black background.
  - Rasterizer default: a single cube, viewed from a camera
    position from which at least one face is visible and at
    least one face is back-facing (so culling can be
    tested).

## 6. Non-goals (honest)

- No OpenGL, Vulkan, Metal, or DirectX, and no GPU code.
- No refraction, no anti-aliasing, no triangle-mesh ray
  tracing, no rasterizer shadows — none of these were seen
  in the verified material, and none are added here.
- No scene-file format, no mesh loading from files, no
  animation loop as a required feature: the book's demos
  are per-algorithm pages, and an animation loop is not a
  stated requirement of either renderer.
- No claim that either sub-build is "the book" on its own:
  Part I alone is not the book, Part II alone is not the
  book, and both together are still a build following the
  book, not the book itself.
