# TEST_PLAN — Computer Graphics from Scratch (Gambetta)

## 0. Pre-code step: toolchain check (mandatory, not optional)

**No JavaScript toolchain is claimed as verified here.**
Before any code is written, the build environment must be
checked and the result recorded:

- Is a JavaScript runtime available (e.g. Node) and at what
  version? Can it run the modules in DESIGN.md?
- Is a browser or a headless canvas available for the demo
  pages, and can an off-screen canvas be read back pixel by
  pixel in tests? If not, what is the test runner instead
  (in-memory pixel buffer with the same `PutPixel`
  interface)?
- What test framework, if any, is used? If none is
  available, the tests must be plain runnable scripts with
  pass/fail output, not a framework that is assumed to
  exist.

If any of these checks fail, that is a blocker to report,
not a detail to work around by dropping pixel tests. The
pixel tests below are the point of this build: a renderer
that is only eyeballed has not been tested.

## 1. Test style

Tests are behaviour-based and pixel-based, in two layers:

- **Unit tests on the math and the algorithms.** Pure
  functions, no canvas needed: vector/matrix operations,
  ray–sphere intersection, shading terms, clipping
  decisions, depth comparisons, texture interpolation.
  These run anywhere JavaScript runs and are the first
  line of defence.
- **Pixel tests on rendered output.** Render a small scene
  to a canvas (or the test pixel buffer), read pixels back,
  and assert on them. Pixel assertions use a tolerance, not
  exact equality, unless stated: floating-point order can
  differ between implementations, but a correct renderer
  must land within a few 8-bit steps of the expected
  colour. Tolerance for colour channels is `+/- 3` (out of
  255) unless a test says otherwise.

No test may assert a value by calling the production code
twice (once to compute the "expected" value, once to
produce the actual value). Expected values in shading and
pixel tests must be computed by hand in the test, or be
qualitative (e.g. "centre pixel is predominantly red",
"this pixel is background", "this pixel is darker than
that pixel"), with the reasoning written in the test.

## 2. Shared-core tests

- **Canvas/PutPixel.** Writing a pixel then reading it back
  returns the colour written. Writing outside the canvas
  bounds does not crash and does not corrupt neighbouring
  pixels (exact behaviour — ignore vs. error — must match
  DESIGN.md).
- **Colour.** Add, scale, multiply, and clamp: clamping
  keeps every channel in range; a colour scaled by 0 is
  black; a colour multiplied by white is unchanged.
- **Math.** Vector add/subtract/scale, dot product
  (perpendicular = 0, parallel = product of lengths),
  length, normalisation (unit length). Matrix multiply:
  identity leaves a point unchanged; a translation matrix
  moves a point by the translation; applying two transforms
  in sequence equals applying their product (order
  documented in DESIGN.md). The coordinate-system
  handedness and matrix layout chosen in DESIGN.md must be
  covered by at least one test that would fail if the
  convention were flipped.

## 3. Part I — Raytracer tests

- **Ray–sphere intersection.** A ray aimed at a sphere's
  centre hits at the expected distance; a ray missing to
  the side has no hit; a ray pointing away has no positive
  hit; the nearest of two spheres on the same ray wins.
- **Basic render (chapter 2).** A scene with one sphere,
  no lights: the centre pixel is the sphere's flat colour,
  a pixel well outside the sphere is the background colour.
- **Light (chapter 3).** Hand-computed cases: a light
  directly overhead gives diffuse near full strength; a
  light behind the surface gives diffuse 0; specular is
  near 0 when the view is far from the reflection
  direction and near full when it is close. The book's
  exponent contrast: the same light and geometry with
  exponent 500 gives a tighter, brighter highlight region
  than with exponent 10 — test the highlight's size, not
  just its existence. Ambient light alone (no point or
  directional light) gives a flat, non-black image.
- **Shadows (chapter 4).** A second sphere placed between
  a hit point and a light removes that light's diffuse and
  specular contribution for the shadowed pixel but leaves
  ambient. A pixel not in shadow is unchanged by adding
  the occluder elsewhere.
- **Reflections (chapter 4).** A reflective sphere
  (reflectivity 1, in a test scene) facing a coloured
  sphere shows the reflected colour, not its own colour,
  within tolerance. A reflectivity-0 material renders
  identically with reflection code on or off. Recursion
  depth: a scene of two facing mirrors terminates — the
  render finishes, it does not recurse forever — and
  increasing the depth limit changes the image only up to
  the limit.
- **Pixel tests (Part I).** Default scene (SPEC section 5),
  small size (e.g. 100×100): centre pixel predominantly the
  centre sphere's colour; a corner pixel is background;
  image dimensions are exactly the requested size. Two
  renders of the same scene are byte-identical at the
  pixel level (the raytracer has no randomness in the
  verified material, so determinism is exact, not
  approximate).

## 4. Part II — Rasterizer tests

- **Lines (chapter 6).** A line from one canvas point to
  another colours exactly the pixels the line algorithm
  covers, including both endpoints; a horizontal and a
  vertical line are covered as special cases, not
  accidents of the diagonal case.
- **Filled triangles (chapter 7).** A triangle fills its
  interior pixels with its colour and does not colour
  pixels outside it. A pixel on a shared edge between two
  adjacent triangles is coloured exactly once (no gaps, no
  double-draw artefacts visible as a colour difference).
- **Shaded triangles (chapter 8).** A triangle with
  different colours at its three vertices shows a smooth
  gradient: pixels near each vertex are close to that
  vertex's colour, and the centre pixel is between them.
- **Perspective projection (chapter 9).** The same object
  placed farther from the camera renders smaller than when
  placed nearer, by the expected ratio within tolerance.
  A point at the camera position or behind it does not
  produce a valid projected pixel (clipping handles it —
  see below).
- **Scene rendering (chapter 10).** The default cube
  scene renders a recognisable cube: at least one face is
  visible, faces have different shades under the scene's
  lights, and the image is not a single flat colour.
- **Clipping (chapter 11).** A cube entirely behind the
  camera renders all background, with no crash and no
  stray pixels. A cube straddling the view boundary renders
  only its visible part. Instance-level clipping (whole
  object out of view) and triangle-level clipping (part of
  an object out of view) are tested as separate cases.
- **Hidden surfaces (chapter 12).** Two overlapping
  triangles, nearer and farther, render the nearer one's
  colour at the overlap pixel regardless of draw order:
  render in both orders and assert the same overlap pixel.
  Back-face culling on: a cube renders fewer visible faces
  than with culling off, and the back faces are absent.
  Back-face culling off: the same cube renders its back
  faces (visible through the front faces' absence of
  culling, per the demo behaviour — to verify against
  chapter 12 before code; if the demo's "off" behaviour
  differs, the test must be rewritten to match it, not
  left asserting an invented behaviour).
- **Shading (chapter 13).** Flat: all pixels of one
  triangle share one colour. Gouraud: pixels vary smoothly
  across a triangle. Phong: per-pixel shading gives a
  highlight that moves correctly with the light position
  (move the light, highlight moves; it does not stay
  glued to a vertex). The three modes on the same scene
  produce three different images.
- **Textures (chapter 14).** A textured triangle shows
  the texture's pattern, not a flat colour. Perspective
  correctness: a textured quad receding into the distance
  shows the texture compressed toward the far edge under
  perspective-correct interpolation, and visibly wrong
  (affine-looking) under linear interpolation — the test
  must distinguish the two modes, since the book's demo
  explicitly toggles them. This is the single most
  important rasterizer test: linear-only interpolation
  must fail it.
- **Pixel tests (Part II).** Default cube scene, small
  size: image dimensions exact; at least one pixel of each
  visible face colour present; background pixels present
  outside the cube. Determinism: two renders of the same
  scene are pixel-identical.

## 5. Comparison test (both renderers, book's own idea)

Render the book's simple scene with both renderers (the
book compares them on the same scene). Assert only what
the book's comparison supports: both produce a
recognisable image of the same scene, with similar overall
layout, while differing in their own strengths — the
raytracer shows shadows/reflections, the rasterizer shows
textures. Do NOT assert pixel equality between the two
renderers: they are different algorithms and must not be
expected to agree pixel for pixel.

## 6. Before any push

- The toolchain check in section 0 is recorded as passed,
  with runtime version and test method named.
- All unit tests and all pixel tests for a part pass before
  that part is presented as done. A part is never presented
  as done on the strength of the other part's tests.
- Any chapter detail marked "to verify before code" in
  SPEC or DESIGN that was verified during development is
  updated in the docs to say so, with the verified value;
  any detail still unverified at push time is listed in
  the push proposal as an open item, not silently dropped.
- Code review against SPEC: every chapter 1–14 feature
  claimed in SPEC is present in the part that claims it;
  no refraction, anti-aliasing, rasterizer-shadow, or
  OpenGL/GPU claim is present anywhere.
