# DESIGN — Scratchapixel Rasterization

## Layout (to be confirmed at code time; no code exists yet)

```
rasterizer.cpp        # single translation unit, or split below
  Vec2 / Vec3         # 2D and 3D vectors: add, sub, scale, dot, cross
  Triangle            # 3 vertices (camera space) + 3 vertex colours
                      #   (+ st coordinates as data, carried but not sampled)
  FrameBuffer         # width x height RGB pixels, initially background
  DepthBuffer         # width x height floats, initially infinity
  project()           # camera -> screen (perspective divide, keep z)
                      #   -> NDC [-1,1] -> raster coordinates
  edgeFunction()      # Pineda edge function for one edge, at one pixel
  barycentric()       # barycentric coordinates from the three edge values
  rasterizeTriangle() # bounding box + edge tests + depth test + colour write
  writePPM()          # frame buffer -> PPM file
  main()              # builds the default scene, rasterizes, writes PPM
```

A single-file layout is acceptable and matches the lesson’s
"learning code, clarity over efficiency" character; splitting
into headers is also acceptable, but is a code-time decision,
not a docs requirement. No external libraries. No image
library: the PPM writer is hand-written.

## Pipeline, in code terms

```
frameBuffer.fill(background)          # e.g. black
depthBuffer.fill(infinity)
for each triangle in scene:
    projected[3] = { project(v) for v in triangle.vertices }
    bbox = clamp(boundingBox(projected), imageBounds)
    for y in bbox.yMin..bbox.yMax:
        for x in bbox.xMin..bbox.xMax:
            e0, e1, e2 = edgeFunction values at pixel (x, y)
            if not inside(e0, e1, e2, topLeftRule): continue
            bary = barycentric(e0, e1, e2)   # sums to 1
            depth = interpolateDepth(bary, projected z values)
            if depth >= depthBuffer[x, y]: continue
            colour = interpolateColour(bary, triangle.vertexColours)
            frameBuffer[x, y] = colour
            depthBuffer[x, y] = depth
writePPM(frameBuffer, outputPath)
```

Notes, each of which must be fixed before code and tested:

- **Pixel centre convention:** edge functions are evaluated
  at pixel centres (x + 0.5, y + 0.5) — or at integer pixel
  coordinates, if the lesson’s code does that. The choice
  changes every expected pixel by up to one row/column, so
  it must be confirmed against the lesson page and then
  frozen in TEST_PLAN’s expected values. Do not mix the
  two conventions between the implementation and the tests.
- **Top-left rule:** a pixel exactly on an edge belongs to
  the triangle only if that edge is a "top" or "left" edge
  (by the lesson’s definition, to be confirmed). This is
  what prevents double-drawing and gaps on shared edges.
- **Depth interpolation:** the depth compared against the
  z-buffer is interpolated from the projected vertices’
  depth values. Whether that interpolation is linear in
  screen space or perspective-correct is a decision that
  must be stated before code — and, for depth, it matters:
  linear screen-space interpolation of depth is wrong
  under perspective. The concluding chapter’s
  perspective-correct interpolation applies to attributes;
  whether the lesson applies it to depth as well is one of
  the items to verify in the Depth Buffer chapter (see
  SPEC §6). Until verified, TEST_PLAN’s depth tests use
  only cases where both conventions agree (e.g. a triangle
  parallel to the image plane, where depth is constant).
- **Colour interpolation:** vertex colours are interpolated
  with the barycentric coordinates, perspective-correctly
  for st coordinates as data; for colour, the lesson’s
  rasterization stage interpolates vertex colour with
  barycentric weights. Whether colour interpolation in the
  lesson is perspective-correct or screen-space linear is
  another confirm-before-code item; TEST_PLAN’s colour
  tests therefore use a triangle parallel to the image
  plane (where both agree) for exact values, and only
  loose bounds elsewhere.
- **Bounding box:** computed from the projected vertices,
  clamped to [0, width) × [0, height). Pixels outside the
  image are never tested — this is both a correctness
  rule (no out-of-bounds writes) and the lesson’s main
  optimisation over the naive whole-image loop.
- **Near clipping:** geometry crossing the near plane must
  be handled before projection produces invalid
  coordinates. The exact procedure is in the un-fetched
  tail of the projection chapter (SPEC §6, item 2); the
  minimum acceptable behaviour for this build is: a
  triangle fully behind the near plane produces no pixels,
  and a triangle crossing it produces no garbage pixels
  (clipped, or conservatively discarded — the choice is
  stated in code review, not silently).

## Numbers and determinism

- All geometry math in `float` or `double` — decide once,
  in code, and keep it consistent; tests must not depend
  on the choice beyond their stated tolerance.
- Depth buffer initial value: infinity (a float/double
  infinity, or the largest representable value — decide
  in code; the test is "a first triangle always wins
  over the background", not the bit pattern).
- No randomness anywhere. The same scene and the same
  image size must always produce a byte-identical PPM.
  Tests may therefore compare PPM files byte-for-byte
  for the deterministic cases, and pixel-by-pixel with a
  tolerance of ±1 per channel for interpolated colours
  (rounding differences only).
- 8×8 pixel blocks and incremental edge-function updates
  and fixed-point coordinates (concluding chapter) are
  optimisations, not correctness requirements. They are
  NOT required for the first working build; they may be
  added as a later ticket only if the pixel tests still
  pass byte-for-byte (or within the stated tolerance)
  afterwards. Clarity over efficiency, per the lesson.

## Failure modes

- Bad arguments (missing output path, non-positive image
  size): usage on stderr, non-zero exit, no file written.
- A write failure (e.g. directory does not exist): error
  message, non-zero exit; a partially written PPM, if any,
  is not presented as a success.
- A scene with no triangles renders all background — that
  is correct, not an error, and is tested as such.
- A triangle fully outside the image, or fully behind the
  near plane, renders nothing and exits successfully.
