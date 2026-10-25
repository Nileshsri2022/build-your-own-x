# TEST_PLAN — Scratchapixel Introduction to Ray Tracing

## Step zero: toolchain check (pre-code, not yet done)

These docs do NOT claim a C++ toolchain in this VM is
verified. Before any code is written or any "tested" claim
is made:

1. Confirm a C++ compiler is present and works
   (e.g. `g++ --version` and a hello-world compile+run).
2. Confirm the C++ standard to target (C++17 proposed;
   confirm the installed compiler supports it).
3. If no compiler is available here, the build/test must
   move to a machine that has one, or a compiler must be
   installed first — and that fact must be recorded in
   the code-review notes. No test below may be reported
   as passing until this step passes.

## Test style

All tests are headless and pixel-based. There is no GUI,
no interactive demo, and no external test framework
prescribed here: small C++ test programs and/or a script
that runs the renderer and inspects the PPM are both
acceptable, as long as the checks below are actually run
and their results recorded. The tutorial is C++, so tests
in C++ (assert-style, exit non-zero on failure) are the
natural fit; a scripted PPM check is required in addition,
not instead, for the integration test.

Pixel assertions use a tolerance, not exact equality,
unless stated: floating-point order can differ by
implementation, but a correct renderer must land within
a few 8-bit steps of the expected colour. Tolerance for
colour channels is `+/- 3` (out of 255) unless the test
says otherwise. Exact assertions (dimensions, header,
file existence, exit codes) have no tolerance.

## Unit tests

- **Vector tests.** Add, subtract, scale, dot (including
  perpendicular = 0, parallel = product of lengths),
  length, normalize (unit length; zero-vector handling
  must be decided in code and documented — it must not
  silently produce NaN pixels).
- **Sphere intersection tests.** Ray straight at a sphere
  centre hits at the expected distance; ray missing to
  the side has no hit; ray tangent has a discriminant of
  (approximately) 0 and one hit; ray starting inside the
  sphere exits at a positive `t`; ray pointing away from
  the sphere has no positive hit; nearest-hit selection:
  two spheres on the same ray, the nearer one wins.
- **Normal test.** At a known hit point, the normal is
  `(hit - centre) / radius`, unit length, pointing
  outward (away from the centre, toward the ray origin
  side for an outside hit).
- **Shadow tests.** A hit point with no blocker between
  it and the light is lit (colour = object colour ×
  brightness, within tolerance); with a second sphere
  between hit point and light, the light contributes 0
  (pixel black, if that was the only light).
- **Reflection / refraction tests (chapter 3 only).**
  A reflective sphere's pixel differs from its flat
  colour in the direction of the reflected scene, within
  tolerance; a glass sphere's pixel is a blend of
  reflected and refracted contributions (exact blend
  values depend on the Fresnel term to verify before
  code — the test must assert the blend structure,
  e.g. "neither purely reflected nor purely refracted
  for a head-on ray", not a formula these docs do not
  state). A depth-limit test: two facing reflective
  spheres render and terminate (process exits, output
  is produced) — recursion without a limit would hang
  and fail this test.
- **PPM writer tests.** Header is exactly `P3`, width,
  height, `255`; body has exactly `width × height × 3`
  integers, each in `[0,255]`; a known 1×1 / 2×2 image
  writes the expected bytes.

## Integration test (scripted, real process)

- Build, run the renderer in a temp directory, producing
  a small PPM (e.g. `200 x 200`).
  - Exit code 0, output file exists, is a valid PPM
    (header `P3`, dimensions match the request, body
    integer count matches).
  - Centre pixel is predominantly the colour of the
    centre sphere of the default scene (within
    tolerance); a corner pixel is background black.
  - A pixel on the reflective sphere differs from that
    sphere's flat colour (reflection is present), and
    a pixel in a sphere's shadow is black (or the
    ambient/background value, if DESIGN adds one —
    currently none is spec'd, so black).
- Empty scene (no spheres, if the CLI/test harness
  supports it, or a test-scene variant): renders all
  background black at the requested size.
- Run with no arguments: exit non-zero, usage on
  stderr, and no output file created.
- Run with a non-positive width or height: exit
  non-zero.

## Before any push

- Toolchain check (step zero) has passed and is recorded.
- All unit + integration tests pass, on the confirmed
  compiler and standard.
- A rendered default-scene PPM exists as a local artefact
  for eyeballing (PPM is text, so it may be pushed only
  if the push procedure and the reviewer explicitly allow
  it — otherwise it stays local, like a PNG would).
- Fresnel formula, refractive index, camera/image-plane
  numbers, and `MAX_DEPTH` are fixed in code review and
  recorded in the code — they must not remain "to verify"
  at push time.
- Code review against SPEC: every chapter 1–3 behaviour
  present, no Phong/textures/anti-aliasing/mesh claims,
  no copied tutorial code (implementation is ours,
  following the concepts).
- Follow-on lesson content is NOT claimed, and the
  README's "to verify before code" list is either
  resolved or carried forward explicitly, not silently
  dropped.
