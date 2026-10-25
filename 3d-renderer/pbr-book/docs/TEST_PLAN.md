# TEST_PLAN — PBR Book, Scoped Subset

## 0. Pre-code toolchain check (must pass first)

**No C++ toolchain is claimed as verified here.** Before
any code is written or any "tested" claim is made:

1. Confirm a C++ compiler is available in the build
   environment, and record its name and version.
2. Confirm how the code will be built (compiler command
   line, or a build system — to be chosen and recorded in
   DESIGN at that point). No build system is claimed in
   these docs.
3. Confirm how tests will be run (test framework, or
   plain assert-based test programs — to be chosen and
   recorded in DESIGN). No framework is claimed here.

If step 1 fails, code work stops and the blocker is
reported; it is not worked around by silently switching
language. The rest of this plan is written so it can be
run with any of those choices.

## 1. Pixel / behaviour tests (the core of this plan)

Pixel assertions use a tolerance, not exact equality,
unless stated: floating-point order can differ by
implementation, but a correct renderer must land within
a few 8-bit steps of the expected colour. Tolerance for
colour channels is `+/- 3` (out of 255) unless the test
says otherwise. Tests that need an exact expected colour
must compute it by hand in the test (not by calling the
production code twice and comparing the code to itself).

- **Geometry (Ch. 3).** Vector add/subtract/scale/dot/
  cross, length, normalisation (unit length; zero vector
  is an error, not silently normalised to NaN — decide
  in code and document it); transformation of points,
  vectors, and normals (normals transform differently
  from points — test that); ray–bounding-box hit/miss,
  including a ray starting inside the box.
- **Monte Carlo (Ch. 2).** Sampling by the inversion
  method reproduces the intended distribution within a
  statistical tolerance over many samples (chi-square
  style check, or mean/variance within tolerance —
  decide at code time, record the tolerance); a
  distribution transform maps samples to the intended
  range.
- **Shapes (Ch. 6).** Sphere: ray at centre hits at the
  expected distance; ray missing to the side has no hit;
  tangent ray has (approximately) one hit; ray from
  inside exits at a positive distance; ray pointing away
  has no positive hit. Cylinder and disk: hit/miss at
  known configurations, including the caps/edges.
  Triangle mesh: a ray through a known triangle hits
  it, a ray through a gap misses, and a shared edge
  between two triangles does not produce a crack
  (no missed pixel along the edge beyond tolerance).
- **BVH (Ch. 7).** On the same scene, BVH intersection
  returns the same nearest hit as brute-force
  intersection for a set of test rays (including rays
  that miss, rays that graze, and rays through dense
  clusters). Rendered image with BVH vs without BVH
  matches within pixel tolerance. Timing: on a scene
  with many primitives, BVH rendering is measurably
  faster than brute force (record both times; no
  speedup number is claimed in advance).
- **Reflection models (Ch. 9).** Diffuse: reflected
  direction distribution is consistent with a diffuse
  (Lambertian-style) model, and evaluated reflectance
  is non-negative and bounded. Specular: a mirror
  reflection direction is the exact reflection of the
  incoming direction about the normal. Conductor:
  reflectance behaves as a metal (tinted, no
  transmission) at a known configuration. Dielectric:
  refraction follows Snell's law at a known angle
  (within tolerance), total internal reflection occurs
  past the critical angle, and the Fresnel term is
  between 0 and 1. Microfacet roughness: increasing
  roughness spreads the reflection (measured as a
  wider distribution of sampled directions, not as a
  single exact value). Measured BSDFs and hair are
  excluded and must have **no** tests claiming them.
- **Cameras and film (Ch. 5).** Projective camera: a
  ray through the image centre points forward, rays
  through the corners point to the corners, and the
  field of view matches the camera parameters. Film:
  N samples per pixel average correctly (a scene
  rendered with N=1 vs N=4 differs only by sampling
  noise, not by a systematic colour shift beyond
  tolerance); the output file has exactly the requested
  dimensions.
- **Path tracer (Ch. 13).** Empty scene renders
  background only. A single diffuse sphere under a
  light renders as a shaded sphere (bright side vs
  dark side, within tolerance, computed by hand in
  the test). A mirror sphere reflects another object
  (reflected colour present, not the sphere's own
  colour, within tolerance). Recursion terminates:
  a scene of two facing mirrors does not hang, and
  finishes within a stated time bound. Determinism:
  two renders with the same seed produce identical
  output files byte-for-byte (for a deterministic
  format such as PPM; if the chosen format is not
  deterministic, pixel-equality within tolerance is
  the criterion, recorded in DESIGN).
- **Rounding error (Ch. 6).** A scene with adjacent
  triangles / a mesh with shared edges renders without
  visible cracks at the shared edges (checked by pixel
  inspection of the rendered image at the edge).

## 2. Integration test (scripted, real process)

- Build, then run the renderer on the default scene
  (fixed at code time, see SPEC §4) at a small size
  (e.g. 100×100).
  - Exit code 0, output file exists, is readable by the
    format's reader, and has the requested dimensions.
  - Centre pixel / a known sphere pixel is not
    background; a corner pixel is background (or as
    the default scene specifies).
- Run with no arguments: exit non-zero, usage on
  stderr, no output file created.
- Run with an unwritable output path: exit non-zero,
  error message, no success claimed.

## 3. Before any push

- Toolchain check (§0) passed and recorded.
- All unit + integration tests pass.
- A rendered default-scene image exists locally for
  eyeballing (not pushed if it is binary — the
  snapshot push used elsewhere in this series is
  UTF-8 only; the image is for the reviewer, not the
  repo, unless the parent explicitly asks otherwise).
- Code review against SPEC: every in-scope chapter
  behaviour present, every out-of-scope item absent
  (no volume scattering, no GPU wavefront, no hair,
  no measured BSDFs, no textures chapter, no scene
  parser), no book code copied, output format and
  colour-model decisions recorded in DESIGN.
- No claim is made about chapters whose bodies were
  not read until their internals are verified against
  chapter text (see README honesty note).
