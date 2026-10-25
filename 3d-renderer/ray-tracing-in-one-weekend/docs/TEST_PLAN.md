# TEST_PLAN — Ray Tracing in One Weekend (Book 1)

## Pre-code step 0: toolchain check (NOT yet verified)

Before any code is written, run and record:

- `g++ --version` (or the available C++ compiler's
  version) in this VM, and a hello-world compile +
  run. The C++ toolchain here is **not verified** at
  docs time, and no test result may be claimed until
  this check passes. If no compiler is available here,
  code is written but marked untested-in-this-VM, and
  tests must be run on a machine with a compiler
  before any "tested" claim or push of code.

All tests below assume that check has passed. Tests
are a mix of small C++ unit-style checks (compiled
with the project headers, no test framework required
by these docs — a plain `assert`-based driver is
acceptable, and the framework choice is a code-time
decision) and scripted PPM checks on real program
output.

## Pixel-based tests (the core of this plan)

PPM parsing for tests: read the `P3` header, width,
height, `255`, then all RGB triples. Pixel assertions
use a tolerance, not exact equality, unless stated:
random sampling means exact bytes vary run to run.
Channel tolerance is +/- 10 (out of 255) for
material/sky colours, +/- 5 for structural checks
(sky vs sphere) where the colours are far apart,
unless a test says otherwise. Use small images and
low sample counts for speed in unit-style checks,
and the book's final parameters only for the final
render check.

- **PPM structure.** Run the program, capture stdout.
  Header is exactly `P3`, then `<width> <height>`,
  then `255`, then exactly width * height RGB triples,
  each in [0, 255]. No extra trailing triples, no
  missing pixels. Tested at 16:9, e.g. 160x90, and at
  a square size, to catch a width/height swap.
- **Empty world = sky gradient.** A world with no
  spheres renders a vertical gradient: top-row pixels
  are near the blue endpoint (0.5, 0.7, 1.0) scaled
  to bytes, bottom-row pixels are near white, and
  every row in between is monotonically between
  them (blue channel non-decreasing top to bottom
  is not required — instead assert top-row blue >
  bottom-row blue by a clear margin, and top-row
  red < bottom-row red, both beyond tolerance).
  Left and right pixels on the same row are equal
  within tolerance (the gradient depends only on
  ray y, not x).
- **Single sphere, centre pixel.** One Lambertian
  sphere centred in view: the centre pixel's colour
  is dominated by the sphere's albedo (e.g. a red
  sphere gives red >> green, blue, beyond tolerance),
  and a corner pixel is sky (matches the empty-world
  gradient for that row within tolerance).
- **Nearest hit wins.** Two spheres on the same ray,
  one nearer, one farther, different albedos: the
  centre pixel matches the nearer sphere's material
  family, not the farther one's.
- **Back face / hollow glass (per TOC, to verify
  before code).** A dielectric sphere renders
  differently from a Lambertian sphere of the same
  colour in the same position: its centre pixel is
  not simply its albedo, because refraction passes
  background light through. Assert "not equal to
  Lambertian centre within tolerance" rather than a
  specific glass colour, since glass colour depends
  on what is behind it.
- **Metal reflection.** A metal sphere in front of a
  coloured background/sphere: the metal sphere's
  pixels reflect the scene behind the camera-side
  view direction, not its own flat colour. In the
  simple test (metal sphere, sky background), the
  sphere's pixels are sky-tinted (bright, blue-ish)
  rather than the metal albedo flat, within
  tolerance. Fuzzy metal (fuzz > 0) blurs that
  reflection: assert its edge pixels differ from
  sharp metal's by more than tolerance, in a
  seeded or averaged comparison (see determinism).
- **Lambertian shading shape.** A Lambertian sphere
  is not a flat disc: pixels near the sphere's edge
  (limb) are darker than pixels near its centre
  facing the camera, because bounce directions there
  sample less sky. Assert centre brightness >
  limb brightness by a clear margin (sum of channels).
- **Anti-aliasing softens edges.** Render the same
  single-sphere scene at 1 sample per pixel and at
  many samples per pixel (e.g. 100). Assert: the
  1-sample image has a hard edge (a pixel row
  crossing the sphere boundary jumps from sky to
  sphere colour in one pixel step), while the
  many-sample image has at least one intermediate
  pixel on that crossing row whose colour is between
  sky and sphere colour beyond tolerance. Flat
  interiors (centre, far corners) differ by no more
  than tolerance between the two renders.
- **Depth limit terminates.** A scene of two facing
  reflective spheres (or a metal sphere facing a
  Lambertian ground, which recurses) renders to
  completion in bounded time and produces no black
  holes larger than expected: assert the program
  exits, the PPM is complete, and no NaN appears
  (parse all triples as integers; a NaN would fail
  parsing or appear out of range).
- **Shadow-acne guard, not shadows.** A Lambertian
  sphere on a ground plane: the contact region does
  not show black speckles (assert no isolated black
  pixels inside the sphere's lit area beyond a small
  count threshold, e.g. fewer than 1% of sphere
  pixels are pure black). Separately, assert there
  is NO cast shadow in this renderer: a sphere does
  not darken the ground behind it beyond what diffuse
  bouncing explains — i.e. ground pixels away from
  the sphere match the no-sphere ground render
  within tolerance. Do not write a test that expects
  a shadow; that would contradict the book.

## Unit-style tests (headers, no rendering)

- **vec3:** add, subtract, scalar multiply/divide,
  dot (perpendicular = 0), cross (right-hand rule),
  length, unit vector (length 1, within 1e-9).
- **ray:** `at(t)` returns origin + t * direction
  for t = 0, 1, and a negative t (no clamping).
- **interval:** contains/surrounds boundaries
  (min/max inclusive for contains, exclusive for
  surrounds), clamp below/above/inside.
- **sphere hit:** ray straight at centre hits at
  the expected t; ray missing to the side misses;
  ray pointing away misses (no positive root in
  interval); ray starting inside exits at positive t;
  nearest root is chosen when two roots exist.
- **material scatter (per TOC, to verify before
  code for metal/dielectric):** Lambertian scatter
  returns true and a non-zero scattered direction;
  metal reflecting below the surface returns false;
  dielectric scatter returns true for a ray hitting
  glass from outside (refraction or reflection,
  either is acceptable — assert only "returns true
  and direction is non-zero").

## Integration test (scripted, real process)

- Build, run `./raytracer > out.ppm` in a temp
  directory (or the book's equivalent binary name,
  recorded at code time). Exit code 0, `out.ppm`
  exists, parses as PPM, dimensions match the
  program's constants, centre pixel is a feature
  sphere colour family, corner pixels are sky.
- Run with stdout redirected to a file vs piped:
  output is identical in structure (no progress
  text mixed into the PPM — progress, if any, goes
  to stderr, per Chapter 2's progress indicator,
  per TOC, to verify before code).

## Determinism — what is and is not claimed

- The book's program, as described, uses unseeded
  standard-library randomness, so two runs differ
  in fine noise. Tests must therefore use tolerance
  and averaged/structural assertions, as above.
- If a seeded mode is added for testing, it must be
  labelled a test-only extension in code and docs,
  and only then may a test assert byte-identical
  PPMs across two runs with the same seed.

## Before any push

- Toolchain check (step 0) passed and recorded.
- All unit-style + pixel + integration tests pass.
- A final render PPM exists locally for eyeballing
  (not pushed — PPM is text, but the final render
  is a large artefact; push only code + docs unless
  the user explicitly asks for the image another way).
- Code review against SPEC: every Book 1 chapter
  behaviour present, no Phong, no lights, no shadow
  rays, no triangles/meshes/textures claimed, no
  book code copied verbatim.
