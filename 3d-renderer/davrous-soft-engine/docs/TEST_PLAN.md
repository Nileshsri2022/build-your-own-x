# TEST_PLAN — 3D Soft Engine (Davrous)

All tests are behaviour and pixel tests on captured back
buffers, plus unit tests for the small pure functions
(interpolation, clamping, line drawing, depth comparison).
No test asserts an internal variable that the user cannot
see in a frame; every behaviour below is visible in pixels.

**§0. Toolchain check — a pre-code step, not an assumption.**
No toolchain is claimed verified for this project. Before
any implementation ticket, confirm, for each language to be
built, and record the result in a short toolchain note
(kept out of the code, alongside these docs):

- C#: a current .NET SDK is installed, a writable-bitmap
  presentation path exists on the target machine, and a
  minimal program can present a coloured back buffer.
- TypeScript: a current TypeScript compiler is installed
  and can compile the demo to plain JavaScript that runs
  in a current browser on a canvas page.
- JavaScript: the demo page runs in a current browser with
  no build step.
- If a language fails this check, it is dropped explicitly
  in a docs revision (SPEC §8), not left half-built. The
  remaining languages still share the same behaviour tests.

Tolerance: pixel tests use a tolerance of ±3 per channel
(out of 255) unless stated, to allow for floating-point
order differences between languages. A test that fails by
more than the tolerance in one language and passes in
another is a bug in the failing implementation, not a
tolerance problem.

## Unit tests

- **Interpolation / clamping.** `Interpolate` at gradient
  0, 0.5, 1 returns the start, midpoint, end values;
  `Clamp` holds values inside the range and passes through
  values already inside it.
- **Line drawing.** Bresenham lines: horizontal, vertical,
  45°, and steep lines draw a continuous line with no gaps,
  from the first endpoint to the last, on a small test
  buffer. (The recursive midpoint method is a stepping
  stone in the tutorial; it is not required to survive into
  the final code, and is not tested as a shipped feature.)
- **Depth comparison.** A nearer triangle drawn after a
  farther one wins at overlapping pixels; a farther
  triangle drawn after a nearer one does not overwrite it;
  the initial depth value and comparison direction are
  pinned by these tests (see SPEC §5 — to verify before
  code, these tests are the verification).
- **Texture wrapping.** Sampling at u = 1.25 returns the
  same texel as u = 0.25; sampling at u = −0.25 returns the
  same texel as u = 0.75. Clamping behaviour, if observed,
  fails this test.

## Behaviour / pixel tests (per language, same scenes)

- **Projection and rotation (Part 1).** The 8 cube vertices
  project inside the frame at a known camera; rotating the
  cube by a fixed step moves its projected vertices in the
  expected direction (handedness/rotation-order check —
  if the cube rotates the wrong way, the matrix order in
  DESIGN is wrong). A cube behind the camera, if tested,
  must not crash the renderer.
- **Wireframe (Part 2).** The cube renders as 12 visible
  edges (allowing for edges hidden only by the frame edge,
  not by any depth logic — wireframe has no depth buffer
  yet). No face is filled; the interior of a face is the
  background colour.
- **Blender import (Part 3).** The Suzanne JSON mesh loads
  without error, has the expected vertex/face counts as
  recorded from the file itself at test time (counts are
  read from the file, not hard-coded from the tutorial),
  and renders as a non-empty, non-background set of pixels
  in the centre of the frame. The torus mesh loads the same
  way. A malformed JSON file fails cleanly (DESIGN).
- **Filled triangles (Part 4).** A single triangle renders
  filled, with no gaps along its edges and no pixels outside
  its edges (checked on a small buffer, pixel by pixel).
  Two overlapping triangles: the nearer one wins at every
  overlapping pixel, regardless of draw order.
- **Flat vs Gouraud (Part 5).** Same scene, flat mode, then
  Gouraud mode:
  - Flat: every pixel of one face has the same shade
    (within tolerance), and two faces at different angles
    to the light have different shades.
  - Gouraud: shade varies smoothly across a face (corner
    pixels differ from centre pixels in the direction the
    normals point), and the face is not flat.
  - Light off / light behind: with the light direction
    reversed, faces go to (approximately) background-dark,
    not negative or wrapped colours.
  - **To verify before code:** the light-direction
    convention and the shade-before/after-texture order
    (DESIGN) — these tests pin them, and the shading
    ticket is not done until they pass in all languages.
- **Textures (Part 6).** A textured triangle shows the
  texture's colours in the right places (a checker texture
  with known texel positions: a pixel mapping to a known
  checker square has that square's colour, within
  tolerance). A texture coordinate outside [0,1] wraps
  (unit test above) rather than clamping or crashing.
  Textured + Gouraud combined: the pixel colour is the
  product of shade and texel, within tolerance — not shade
  alone, not texel alone.
- **Back-face culling (Part 6).** A closed mesh (cube)
  rendered with culling on shows the same visible faces as
  with culling off, pixel for pixel within tolerance, and
  renders no fewer front-facing pixels. This is the test
  for the tutorial's reported culling bug: if culling
  removes a front-facing triangle, the test fails and the
  culling implementation must be fixed or the feature
  disabled, not shipped with the bug.
- **Empty scene.** No meshes → a cleared back buffer, no
  crash, in all languages.
- **Cross-language agreement.** The same scene, same frame
  number, rendered by C#, TypeScript, and JavaScript (each
  language that passed §0), must agree pixel for pixel
  within tolerance. Any larger difference is a bug.

## Performance tests (Part 4b, C# only, informational)

- Frame rate is measured, not asserted as a pass/fail
  threshold: the tutorial's numbers (45 → 40 FPS for
  per-scanline parallelism, ~53 FPS for per-triangle with
  per-pixel locks, 15 → 25 FPS for culling, all in IE11
  in 2013) are historical claims, not targets. No modern
  FPS number is promised here.
- The only hard performance rule: the parallel version
  must produce identical pixels to the serial version for
  the same frame. Speed without that is a failure.
- HTML5/Web Workers parallelism is not tested because it
  is not built (SPEC §6).

## Before any push

- §0 toolchain check recorded, with any dropped language
  documented in a docs revision.
- All unit + behaviour + cross-language tests pass, for
  every language that passed §0.
- A captured frame (or a small set of frames: cube
  wireframe, cube filled, Suzanne shaded, torus textured)
  exists locally for eyeballing — as image files produced
  by the test harness from back buffers, not as a feature
  of the engine, which has no image-file output.
- Code review against SPEC: every Part 1–6 behaviour
  present, no out-of-scope feature claimed (no shadows,
  no Phong, no triangle clipping, no WebGL engine, no
  engine anti-aliasing), no tutorial code copied verbatim.
