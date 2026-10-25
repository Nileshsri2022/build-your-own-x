# TEST_PLAN — Rogach Pure Java 3D Render Engine

All tests are JUnit 5, run headlessly. No test opens a
Swing window: tests call `Renderer.render` directly and
inspect the returned `BufferedImage` pixel by pixel.
Pixel assertions use a tolerance, not exact equality,
unless stated: floating-point order can differ by
implementation, but a correct renderer must land within
a few 8-bit steps of the expected colour. Tolerance for
colour channels is `+/- 3` (out of 255) unless the test
says otherwise. Colours are compared per channel after
the sRGB shading step, so expected values in tests must
be computed through the same 2.4-exponent approximation
documented in SPEC — a test that expects naive
multiply-only shading is wrong, not the code.

## Unit tests

- **Matrix3Test.** Identity leaves a vertex unchanged;
  a 90-degree XY rotation maps (1,0,0) to (0,1,0)
  (within 1e-9); a 90-degree XZ rotation maps (1,0,0)
  to (0,0,-1) under the article's matrix convention
  (test asserts the article's convention as implemented,
  and names it if it differs from a textbook rotation);
  matrix multiplication composes: applying
  heading-then-pitch as one multiplied matrix equals
  applying the two matrices in sequence, vertex by
  vertex; rotating by 0 degrees in both axes is the
  identity.
- **RasterTest (barycentric coverage).**
  - A triangle rendered at a known position covers
    exactly the pixels its barycentric test admits:
    a pixel at the triangle's centroid is coloured,
    a pixel clearly outside all three edges is not
    (still background/transparent).
  - A pixel on an edge is either coloured or not, but
    the test must not depend on which: edge pixels are
    asserted only as "coloured or background", never
    as a hard colour, unless the implementation's edge
    rule (weights in [0,1] inclusive) is the thing
    being tested — in that case the test names the
    rule.
  - Bounding-box clamping: a triangle partly outside
    the image renders its in-bounds part without
    throwing and without writing outside the image
    (asserted by rendering at the image edges and
    checking no exception and a correct in-bounds
    region).
  - Degenerate triangle (area 0): renders nothing,
    throws nothing, and leaves the z-buffer and
    neighbouring pixels untouched.
- **Depth-ordering test (z-buffer).**
  - Two overlapping triangles, the nearer one (larger
    z toward the viewer) listed second: the overlap
    pixels show the nearer triangle's colour, not the
    last-listed one's. Then the same pair with depths
    swapped: the overlap flips to the other colour.
    This catches both "last painted wins" (no
    z-buffer) and a reversed comparison (farthest
    wins).
  - A triangle fully behind another (all its pixels
    at smaller depth) contributes no visible pixels.
- **ShadeTest (flat shading).**
  - A triangle facing the light (normal (0,0,1)):
    shade coefficient 1, colour unchanged (within
    the sRGB round-trip tolerance).
  - A triangle facing sideways (normal (1,0,0)):
    shade coefficient 0, colour black (within
    tolerance) — the |norm.z| rule, not a sign rule.
  - A triangle at 45 degrees to the light: shade
    strictly between the facing and sideways cases,
    and approximately cos(45°) after the sRGB
    round-trip (tolerance widened to +/- 10 for this
    case, because the 2.4 approximation is itself
    approximate).
  - Shading follows rotation: the same triangle
    rendered at heading 0 and at heading 90 has
    different shades (the normal is computed from
    transformed vertices). A test that shades from
    untransformed vertices fails this.
  - `getShade` with shade 0 gives black, with shade
    1 gives the original colour, for at least one
    non-grey colour (so a channel-swapping bug is
    caught).
- **InflateTest (sphere approximation).**
  - One round on the tetrahedron: triangle count
    goes from 4 to 16; two rounds: 16 to 64.
  - After inflating, every vertex lies on the sphere
    (distance from the origin equals the sphere
    radius within 1e-6) — not just "more triangles".
  - Colours are inherited: each inflated patch keeps
    its parent triangle's colour.
  - Zero rounds is the identity: same triangles,
    same colours, same vertices.
- **RendererTest (small, in-memory).**
  - Empty triangle list renders a blank image of
    exactly the requested size.
  - Tetrahedron, heading 180 / pitch 0, small size
    (e.g. 200×200): the image is non-blank, contains
    at least two of the four face colours in distinct
    regions, and the centre region is coloured (the
    tetrahedron is centred at the origin, so it
    covers the centre at this rotation).
  - Determinism: two renders with the same triangles,
    heading, pitch, and size produce pixel-identical
    images (no randomness anywhere in this renderer).
  - Flat shading is per triangle, not per pixel:
    all pixels of one visible face share the same
    colour (within tolerance) — a gradient inside a
    face would be Gouraud shading and fails this.

## Integration test (scripted, real process)

- Build, run `Main out.png` (our PNG addition) in a
  temp directory.
  - Exit code 0, `out.png` exists, is a valid PNG
    (readable by `ImageIO`), and has the requested
    dimensions (default 400×400 if no size given).
  - The PNG is non-blank and contains at least two
    face colours.
- Run `Main` with a bad size argument (e.g. width 0):
  exit non-zero, usage/error on stderr, and no
  `out.png` created.
- The Swing window itself is NOT asserted in automated
  tests (a window cannot be checked headlessly); it
  is checked manually once, by opening it and moving
  both sliders, and the result is recorded as a manual
  check, not as a passing automated test.

## Before any push

- All unit + integration tests pass, on the Java 11
  target.
- A rendered PNG of the tetrahedron (and one of the
  inflated sphere) exists as a local artefact for
  eyeballing (not pushed — PNGs are binary and the
  snapshot push is UTF-8 only; the PNG is for the
  reviewer, not the repo, unless explicitly asked for
  another way).
- Code review against SPEC: orthographic only, flat
  shading only, triangles only, no textures, shadows,
  reflections, or perspective claims; no copied
  article code (implementation is ours, following
  the article's sections); no gist URL cited anywhere.
