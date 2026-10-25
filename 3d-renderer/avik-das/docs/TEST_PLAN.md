# TEST_PLAN — 3D Renderer

All tests are JUnit 5, run headlessly (no window, no browser).
Pixel assertions use a tolerance, not exact equality, unless
stated: floating-point order can differ by implementation, but a
correct renderer must land within a few 8-bit steps of the
expected colour. Tolerance for colour channels is `+/- 3` (out
of 255) unless the test says otherwise.

## Unit tests

- **Vec3Test.** Add, subtract, scale, dot (including
  perpendicular = 0, parallel = product of lengths), cross
  (right-hand rule), length, normalize (unit length, zero vector
  is an error / not silently normalised to NaN — decide in code
  and document it), lerp at 0 / 0.5 / 1.
- **SphereTest.** Ray straight at a sphere centre hits at the
  expected distance; ray missing to the side has no hit; ray
  tangent has a discriminant of (approximately) 0 and one hit;
  ray starting inside the sphere exits at a positive `t`;
  ray pointing away from the sphere has no positive hit;
  nearest-hit selection: two spheres on the same ray, the
  nearer one wins, and its normal points outward.
- **ShadingTest.** Phong at a known configuration, computed by
  hand in the test (not by calling the production code twice):
  a sphere hit with light directly overhead gives diffuse
  near full strength; light behind the surface gives diffuse
  0; specular is 0 when the view is not near the reflection
  direction and near 1 when it is; shadow: a second sphere
  between the hit point and the light removes diffuse and
  specular but leaves ambient.
- **RendererTest (small, in-memory).**
  - Empty scene renders all background (black) at any size.
  - Default scene, `S = 1`, small size (e.g. `40 x 40`):
    centre pixel is predominantly red (centre sphere), a
    pixel well outside all spheres is background black, and
    the image dimensions are exactly the requested size.
  - `S = 1` vs `S = 4` on the same scene: the images differ
    only at edges (anti-aliasing changes edge pixels, not flat
    interiors by more than the tolerance).
  - Determinism: two renders with the same seed and samples
    produce byte-identical PNGs.
  - Reflection: a scene with one reflective sphere (`k = 1`)
    facing a coloured sphere behind the camera-side view
    shows the reflected colour, not the sphere's own colour
    (within tolerance) — or, if that setup is fiddly, a
    direct `trace` unit test on the reflection blend formula
    `(1-k)*local + k*reflected` is acceptable and must be
    stated as such in the test name.

## Integration test (scripted, real process)

- Build, run `Main out.png 100 100 4` in a temp directory.
  - Exit code 0, `out.png` exists, is a valid PNG
    (readable by `ImageIO`), and is `100 x 100`.
  - Centre pixel is predominantly red; a corner pixel is
    background black.
- Run `Main` with no arguments: exit non-zero, usage on
  stderr, and no `out.png` created.
- Run `Main out.txt`: exit non-zero (bad extension).

## Before any push

- All unit + integration tests pass, on the Java 11 target.
- A rendered `400 x 400` default scene PNG exists as a local
  artefact for eyeballing (not pushed — PNGs are binary and the
  snapshot push is UTF-8 only; the PNG is for the reviewer, not
  the repo, unless you explicitly ask for it another way).
- Code review against SPEC: every lesson 1–6 behaviour present,
  no triangle/mesh/texture claims, no copied tutorial code
  (implementation is ours, following the lessons).
