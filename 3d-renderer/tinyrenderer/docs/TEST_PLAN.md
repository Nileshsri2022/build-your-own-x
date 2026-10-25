# TEST_PLAN — tinyrenderer

## Pre-code step (must pass before any code is written)

- **Toolchain check.** Confirm a C++ compiler and CMake
  are available in the environment where the build will
  be tested, and record their versions. This is NOT
  verified at docs time. If no C++ toolchain is
  available here, the build/test must move to a machine
  that has one, and no test result is claimed until
  that run actually happens. Do not claim "C++ verified"
  on the basis of these docs.

## Test style

All checks are pixel-based, on real renders, not on
internal function calls alone. Where a check inspects
pixels, it reads the output TGA (through the TGA class
or an equivalent reader) and asserts on pixel colours
or on file properties. Colour assertions use a
tolerance, not exact equality, unless stated:
floating-point order and compiler differences can
shift a channel by a few steps, but a correct render
must land within a few 8-bit steps of the expected
colour. Tolerance for colour channels is `+/- 3`
(out of 255) unless the test says otherwise.

## Pixel / file tests (in ticket order)

- **Framebuffer / pixels.** Render with no model (or
  with a model entirely off-screen): output file
  exists, is a valid TGA, has the expected dimensions,
  and every pixel is the background colour.
- **Triangle rasterization.** Render a single
  triangle of a known solid colour, facing the camera:
  centre pixels inside the triangle are that colour
  (within tolerance), pixels well outside it are
  background, and the triangle's edges are where the
  geometry says they should be (within 1 pixel).
- **Back-face culling.** Render the same triangle
  with reversed winding (facing away): the output is
  all background. Render it facing the camera again:
  it appears. Both must hold.
- **Hidden-surface removal.** Render two overlapping
  triangles, one nearer than the other: the nearer
  triangle's colour wins at every overlapping pixel,
  regardless of the order the triangles are submitted
  in (run both orders).
- **Camera / projection.** Render a known mesh at two
  camera positions (or two projection settings, as
  fixed by the camera ticket): the rendered object
  moves / scales in the direction the geometry
  predicts, and does not vanish or invert unexpectedly.
  Exact pixel values are not asserted here — position
  and size, within a few pixels, are.
- **Shading (Phong).** Render a known triangle/mesh
  with a known light: a face angled toward the light
  is brighter than the same face angled away, and a
  specular highlight appears where the reflection
  direction and view direction align (within the
  tolerance, and as a relative comparison, not an
  absolute colour claim, until the shading ticket
  fixes the coefficients).
- **Textures.** Render a mesh with a known texture
  (e.g. a texture whose quadrants are four distinct
  colours): the rendered pixels show the texture's
  colours in the order the UV mapping predicts,
  not a solid colour and not a scrambled mapping.
- **Normal mapping.** Render the same mesh with and
  without its normal map: the two images differ in
  shading (per-pixel variation where the normal map
  varies), while the silhouette and the texture
  colours stay the same within tolerance.
- **Shadow mapping.** Render a scene with an occluder
  between the light and a surface: pixels on the
  surface behind the occluder are in shadow (dark,
  ambient-only level within tolerance) and pixels
  outside the occluder's shadow are lit, with a hard
  edge between them (no gradual falloff band wider
  than 1–2 pixels, matching the hard binary mask).
- **Determinism.** Render the same inputs twice:
  the two TGA files are byte-identical.
- **Full render.** Render a real triangulated `.obj`
  mesh with textures (the tutorial repo's example
  models, if available, or a small equivalent mesh
  created for testing): output file exists, is a
  valid TGA of the expected dimensions, is not all
  background, and is not all one colour.

## Integration test (scripted, real process)

- Build with CMake, run the program on a small test
  mesh in a temp directory.
  - Exit code 0, output TGA exists, is valid, and has
    the expected dimensions.
  - The rendered image is not all background.
- Run with no arguments: exit non-zero, usage on
  stderr, and no output file created.
- Run with a missing model file: exit non-zero, an
  error on stderr, and no output file presented as
  success.

## Before any push

- All pixel + integration tests pass, on a verified
  C++ toolchain (see pre-code step).
- A rendered TGA of a real mesh exists as a local
  artefact for eyeballing (not pushed — TGA is a
  binary format and the snapshot push is UTF-8 only;
  the TGA is for the reviewer, not the repo, unless
  explicitly asked for another way).
- Code review against SPEC: every verified feature
  present, no ray-tracing / GPU / animation claims,
  no copied tutorial code (implementation is ours,
  following the course notes), and every "to verify"
  item in README/SPEC either resolved and recorded
  or still explicitly marked as unresolved.
