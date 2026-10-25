# TEST_PLAN — Scratchapixel Rasterization

All tests are pixel-based: they read the program’s PPM
output and check pixels, dimensions, and ordering. There is
no window to look at, and no "looks right" test — every
assertion is a number.

## 0. Pre-code step (not a test, a gate)

- **Toolchain check.** Confirm a C++ compiler is available
  in the build environment and can compile a trivial
  program, and record the compiler and its version. The
  C++ toolchain in this VM is **NOT verified** — do not
  claim any build or test is verified until this check
  passes. If no compiler is available here, the build
  moves to a machine that has one, and that fact is
  recorded; no test is reported as passing on a machine
  that cannot compile the code.
- **Unverified-chapter check.** Before code, re-fetch the
  Depth Buffer chapter and the tails of the projection,
  rasterization, and concluding chapters (SPEC §6), and
  record what they say. Any test whose expected value
  depends on an unverified detail is marked as such until
  that fetch lands.

## 1. PPM format tests

- Output file exists, and its header is exactly the variant
  chosen in DESIGN (P3 or P6), with the requested width,
  height, and max colour value.
- The pixel data has exactly width × height pixels, in
  row-major order, top row first (verified by a scene
  whose top and bottom rows differ).
- An empty scene renders all background colour at any size.
- Bad arguments (no output path, width or height ≤ 0)
  exit non-zero, print usage to stderr, and create no file.

## 2. Triangle coverage tests

- **Single triangle, known geometry.** A triangle with
  vertices at known raster coordinates, all the same
  colour, rendered at a small size (e.g. 32×32). Every
  pixel strictly inside the triangle is that colour;
  every pixel strictly outside is background. Pixels
  exactly on an edge are checked against the top-left
  rule, not against a guess.
- **Bounding-box correctness.** A small triangle in one
  corner of a large image: pixels outside its bounding
  box are untouched background, and the run touches no
  pixel outside the box (checked by output equality with
  a reference render, or by a test build that counts
  tested pixels — the mechanism is a code-time decision,
  the property is not optional).
- **Clipping to the image.** A triangle that extends
  beyond the image edges renders only its in-image part,
  with no crash and no out-of-bounds write, and the
  in-image pixels match the same triangle rendered
  fully in-image.
- **Shared edge (top-left rule).** Two triangles sharing
  an edge, with different colours, rendered together:
  every pixel on the shared edge has exactly one of the
  two colours (never a blend, never background, never
  double-drawn in a way that changes the result), and
  the union of the two triangles covers the quad with
  no gap and no overlap artefact.

## 3. Barycentric interpolation tests

- **Vertex colours.** A triangle with three different
  vertex colours (e.g. red, green, blue), parallel to the
  image plane (so screen-space and perspective-correct
  interpolation agree — see DESIGN). The pixel nearest
  each vertex is (within ±1 per channel) that vertex’s
  colour; the pixel at the triangle’s centroid is (within
  ±1 per channel) the average of the three vertex colours.
- **Centroid check, stated tolerance.** Interpolated
  colours are asserted within ±1 per 8-bit channel, to
  allow for rounding-order differences. Exact equality is
  required only for solid-colour triangles and for
  background pixels.

## 4. Depth ordering tests (z-buffer)

- **Nearer wins.** Two overlapping triangles, one nearer
  to the camera than the other, different colours, both
  parallel to the image plane (so depth-interpolation
  convention does not affect the result). In the overlap
  region, every pixel is the nearer triangle’s colour,
  regardless of the order the triangles are submitted.
  Rendered twice, with the submission order swapped, the
  two PPM files are byte-identical.
- **Farther does not overwrite.** The same pair, submitted
  nearer-first and farther-first: identical output in both
  cases (this is the z-buffer property, not a submission-
  order property).
- **Equal depth.** Two coplanar overlapping triangles:
  the result is deterministic (same PPM on repeated runs),
  and the chosen winner rule (first-wins vs last-wins,
  decided in code) is stated in the code review — not
  left as an accident of floating-point comparison.
- **Depth-buffer chapter caveat.** Any depth test that
  depends on depth interpolation under perspective (a
  slanted triangle) is marked UNVERIFIED until the Depth
  Buffer chapter is fetched (SPEC §6, item 1). Such tests
  may be added after that fetch, not before.

## 5. Projection tests

- **Perspective size.** The same triangle rendered at two
  depths: the nearer one covers more pixels than the
  farther one, by the perspective ratio (within one pixel
  of rounding at the edges). This is a pixel-count check,
  not an eyeball check.
- **Off-centre triangle.** A triangle displaced sideways
  in camera space projects to the corresponding side of
  the image (left/right, up/down) — checked by the
  centroid pixel position of the rendered triangle.
- **Behind the near plane.** A triangle fully behind the
  near plane renders no pixels and exits successfully.
  A triangle crossing the near plane renders no garbage
  pixels outside its valid projected region (clipped or
  conservatively discarded — see DESIGN).

## 6. Determinism and regression

- The same scene rendered twice produces byte-identical
  PPM files.
- The default single-triangle scene, rendered at the
  fixed default size, is kept as a reference PPM (local
  artefact for review, not necessarily pushed — PPM is
  text in P3 form, so it may be pushed if the user asks;
  in P6 form it is binary and follows the snapshot
  UTF-8-only rule, i.e. it is not pushed).
- No performance test is required: the lesson itself says
  the code is for learning, clarity over efficiency.
  A render that takes seconds at the test sizes is fine.

## Before any push

- Toolchain check (§0) passed and recorded.
- All tests in §§1–6 pass, on the chosen compiler.
- Code review against SPEC: triangles only, no shading
  model claimed, no texture sampling claimed, no
  anti-aliasing claimed, Depth Buffer chapter status
  (verified or still unverified) stated in the review.
- No lesson code copied verbatim: the implementation is
  ours, following the lesson’s algorithm.
