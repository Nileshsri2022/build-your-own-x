# DESIGN — tinyrenderer

## Layout (proposed, no code yet)

```
CMakeLists.txt
src/
  main.cpp       # CLI parsing, load models, run pipeline, write TGA
  tgaimage.h     # supplied TGA-image class (tutorial-provided, not ours to re-derive)
  tgaimage.cpp
  model.h        # .obj mesh: vertices, texture coordinates, faces, textures
  model.cpp
  geometry.h     # vectors, matrices, barycentric helpers (our implementation)
  renderer.h     # framebuffer, depth buffer, triangle rasterization, shading
  renderer.cpp
  shaders.h      # shading functions incl. Phong, normal mapping, shadow test
tests/
  (pixel-based checks, see TEST_PLAN — no unit-test framework is
   specified here; the checks are small C++ programs / scripts
   that render and inspect TGA pixels, so they need no framework)
```

Module names follow the concepts in SPEC. The tutorial
repo's file names (`main.cpp`, `model.cpp/h`,
`geometry.h`, `our_gl.cpp/h`, `tgaimage.cpp/h`) are
similar in spirit, but this layout is our own plan;
it is not a claim that the tutorial uses exactly
these files. If the code ticket finds the notes use a
different split, the ticket records the change.

## Data

- **Mesh.** Vertices (3D points), texture coordinates
  (2D, per vertex), faces (triples of vertex indices,
  triangulated by the input file — the loader does not
  triangulate polygons itself; a non-triangulated face
  is an input error, reported, not silently fixed).
- **Texture.** An image loaded through the TGA class,
  sampled by interpolated UV coordinates.
- **Framebuffer.** A 2D array of TGA colours, one per
  output pixel, initially a background colour.
  The tutorial's starting point sets individual pixels;
  every later stage writes through this buffer only.
- **Depth buffer.** A 2D array of depths, one per
  pixel, initialised to "infinitely far". A fragment
  writes its colour only if its depth is nearer than
  the stored depth, and then updates the stored depth.
- **Shadow depth buffer.** A second depth buffer,
  filled by the light-view pass (SPEC step 8), and
  read (not written) by the camera pass.

## Rendering loop

```
load models + textures
framebuffer = background
depthbuffer = infinity
for each triangle in each model:
  transform its 3 vertices to screen space
  if signed-area test says back-facing: skip
  compute the triangle's screen bounding box, clamped to the framebuffer
  for each pixel in the bounding box:
    compute barycentric coordinates
    if pixel is outside the triangle: skip
    interpolate depth, UV, normal from the 3 vertices
    if depth is farther than depthbuffer[pixel]: skip
    depthbuffer[pixel] = depth
    colour = shade(interpolated data, texture, normal map, shadow buffer)
    framebuffer[pixel] = colour
write framebuffer to framebuffer.tga
```

Shadow mapping wraps this loop: run the loop once
from the light's viewpoint writing only the shadow
depth buffer (no colour output), then run it again
from the camera, testing each fragment against the
shadow depth buffer to decide the hard shadow mask.

## Numbers and determinism

- All geometry math in `double` (or `float` if the
  shading ticket records a reason — the choice must
  be recorded there, not assumed here).
- No randomness is specified. The same input models
  and textures must produce the same TGA bytes on
  every run (TGA encoding of the same pixels is
  deterministic). Tests must not depend on floating
  point bit-equality across compilers; pixel tests
  use a tolerance (see TEST_PLAN).
- TGA: RGB (or RGBA where the input requires it),
  written by the supplied TGA class. No gamma
  correction unless SPEC §4 is revised by the
  shading ticket with a verified source.

## Failure modes

- Missing / unreadable `.obj` or texture file: error
  + exit non-zero, no output file presented as success.
- Non-triangulated face: input error, reported, not
  silently triangulated.
- A model that projects entirely off-screen renders
  an all-background image — that is correct, not an
  error, and is tested as such.
- A write failure (e.g. output directory does not
  exist): error message + non-zero exit; a partially
  written TGA, if any, is not presented as success.
