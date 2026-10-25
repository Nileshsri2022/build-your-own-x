# DESIGN — Scratchapixel Introduction to Ray Tracing

## Layout (proposed, C++, standard library only)

```
main.cpp            # CLI, builds the default scene, renders, writes the PPM
                    # (single-file layout is deliberate: the tutorial's
                    # example is a few hundred lines, and splitting it
                    # across many headers would add structure the
                    # tutorial does not teach; splitting is a code-review
                    # decision, not a docs promise)
```

If code review splits the file, the expected units are:

```
Vec3       # immutable 3D vector: add, sub, scale, dot, length, normalize
Ray        # origin + direction, pointAt(t)
Sphere     # centre, radius, colour, material kind; intersect(ray) -> Hit or none
Material   # kind: opaque / reflective / glass; colour; reflectivity / refractive index
Light      # position, brightness/colour
Scene      # spheres, lights, background, defaultScene()
Renderer   # render(scene, width, height, maxDepth) -> image buffer
PpmWriter  # write a P3 PPM file from an image buffer (thin wrapper, for tests)
```

Names follow the concepts in SPEC. The tutorial's own
identifiers were not copied from a code block that was not
verified here (see README), so names are ours.

## Rendering loop

```
for each pixel (row y, column x):
  ray = primary ray from the eye through the centre of pixel (x, y)
  colour = trace(ray, depth = 0)
  image[x][y] = colour
write PPM
```

`trace(ray, depth)`:

```
hit = nearest sphere intersection with t > EPSILON (1e-4)
if hit is null: return background
if hit object is the visible light-source sphere: return its colour
colour = black
for each light:
  if any sphere is hit by a shadow ray from (hit.point + normal * EPSILON)
     toward the light, with t < distance to light:
    continue   # light blocked
  colour += object.colour * light.brightness
if material is reflective or glass and depth < MAX_DEPTH (e.g. 5):
  reflected = trace(reflected ray, depth + 1)
  refracted = trace(refracted ray, depth + 1)   # glass only
  colour = blend(colour, reflected, refracted, fresnelTerm)
  # exact blend formula and Fresnel term: TO VERIFY BEFORE CODE
  # from chapter 3; do not hard-code a formula these docs do not state
return colour.clamped()
```

Notes:

- Primary rays: one per pixel, through the pixel centre.
  No jittered samples, no anti-aliasing — the read chapters
  do not teach it, and adding it would change every
  expected pixel.
- The shadow test uses the same nearest-intersection code
  as primary rays, with a maximum distance (the light
  distance). A transparent sphere blocks light in this
  scope — a documented limitation in SPEC, not a bug to
  fix silently.
- Recursion: depth-limited. Beyond `MAX_DEPTH`, the
  reflected/refracted contribution is the background
  colour. The exact `MAX_DEPTH` value is fixed in code
  review and recorded in the code, not asserted here as
  tutorial-given.

## Numbers and determinism

- All math in `double`. Epsilon `1e-4` for self-intersection
  and shadow origins, and for rejecting hits behind / at
  the ray origin (`t > EPSILON`, not `t > 0`). This epsilon
  is an implementation choice, not a tutorial-prescribed
  constant — it is stated here so tests can rely on it.
- No randomness anywhere in the read-chapters scope, so a
  render is deterministic: the same program and scene
  always produce the same PPM bytes (given the same
  formatting: one integer per channel, whitespace as
  written by `PpmWriter`).
- PPM: ASCII `P3`, header `P3`, width, height, max value
  `255`, then decimal RGB triples. No gamma correction,
  no tone mapping — the read chapters teach none, and
  adding either would silently change every expected
  pixel.

## Camera / image plane (to confirm in code review)

The read material establishes the concepts (eye origin,
image plane, one ray per pixel centre, perspective
projection) but the exact plane placement, field of view,
and aspect handling were not all verified from the text
fetch. DESIGN therefore does not fix numbers here.
Before code, the implementer must fix, in code review:
eye position, image-plane distance and extent, how width
vs height maps to the plane (square test images avoid
aspect questions; non-square renders must define it),
and the forward axis. Those values then become part of
the test expectations in TEST_PLAN — they must not be
invented mid-implementation without recording them.

## Failure modes

- Unknown output path, non-positive width/height, or a
  missing output argument: usage + exit 1, no file
  written (or no partial file presented as success).
- A write failure (e.g. directory does not exist): the
  error is printed and the process exits non-zero.
- A scene with no spheres renders all background — that
  is correct, not an error, and is tested as such.
- A reflective/glass sphere facing another reflective/
  glass sphere must terminate: if `MAX_DEPTH` is not
  enforced, that is a bug, and the depth-limit test in
  TEST_PLAN catches it.
