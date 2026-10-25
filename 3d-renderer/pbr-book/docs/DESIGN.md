# DESIGN — PBR Book, Scoped Subset

## Status of this design

This layout is a **proposal** following the scoped chapters'
structure (Ch. 1–7, 9, 13, per contents page). It is not the
book's code, not copied from pbrt, and not verified against
the chapter bodies. Before code, two DESIGN decisions must
be pinned down and written here: (1) the output format
(PPM proposed, see SPEC §5), and (2) the colour model
(full spectral vs RGB approximation, see SPEC §2, Ch. 4).
Code must not start until both are recorded.

## Proposed module layout (C++)

```
src/
  geometry.h/.cpp    # Ch. 3: vectors, points, normals, rays,
                     #   bounding boxes, transformations,
                     #   surface interactions (names are ours)
  montecarlo.h/.cpp  # Ch. 2: random sampling, inversion method,
                     #   distribution transforms, as far as Ch. 13 needs
  radiometry.h/.cpp  # Ch. 4: spectrum / colour type (per decision above),
                     #   radiometric quantities used by BSDFs
  camera.h/.cpp      # Ch. 5: camera interface, projective camera;
                     #   spherical camera as stretch goal only
  film.h/.cpp        # Ch. 5: pixel accumulation, image output
                     #   (format decided at code time; PPM proposed)
  shapes.h/.cpp      # Ch. 6: shape interface; sphere, cylinder, disk,
                     #   triangle mesh (must-have); bilinear patch,
                     #   curve (stretch goals); rounding-error handling
  primitives.h/.cpp  # Ch. 7: primitive interface, aggregates,
                     #   BVH construction and traversal
  bsdf.h/.cpp        # Ch. 9: BSDF interface (sample / evaluate / PDF);
                     #   diffuse, specular, conductor, dielectric,
                     #   microfacet roughness, rough dielectric
                     #   (measured BSDFs and hair: excluded)
  integrator.h/.cpp  # Ch. 13: light transport equation, simple
                     #   path tracer (target); better path tracer
                     #   (stretch goal only)
  main.cpp           # CLI: scene + camera + sampler settings,
                     #   output path; builds the default scene
                     #   (default scene fixed at code time, see SPEC §4)
tests/
  # one test file per module above, plus integration tests
  # (see TEST_PLAN); test framework to be chosen at code time
  # and recorded here — no framework is claimed in these docs
```

No file above is claimed to match pbrt's file layout; the
grouping follows chapter topics so a reader can map each
module back to its chapter.

## Rendering loop (proposed, to verify against Ch. 13)

```
for each pixel (x, y):
  colour = black
  for each of N samples:
    ray = camera.generate_ray(x, y, sample)
    colour += path_trace(ray, depth = 0)
  film.add_sample(x, y, colour / N)
film.write(output_path)
```

`path_trace(ray, depth)` (proposed):

```
hit = bvh.intersect(ray)          # nearest hit, or miss
if hit is null: return background / environment (verify: what
  does Ch. 13's simple tracer return on a miss?)
if depth >= MAX_DEPTH: return black (verify: depth value and
  termination rule — Russian roulette or fixed depth — from Ch. 13)
bsdf = hit.material.bsdf
sampled_direction, throughput = bsdf.sample(hit, incoming_ray)
  # per Ch. 9 BSDF interface, verified before code
return throughput * path_trace(reflected_ray, depth + 1)
  + direct_light_contribution(hit)   # only if Ch. 13's simple
  #   tracer includes it; verify — do not add a direct-light
  #   sampler from Ch. 12 silently
```

All of the above (MAX_DEPTH, N, miss colour, direct-light
handling) are Ch. 13 values to verify, not invented
constants. Record the verified values in this file before
code.

## BVH behaviour

- Construction: partition primitives by bounding box
  (exact split rule to verify against Ch. 7; record it
  here before code).
- Traversal: a ray must return the same nearest hit as
  brute-force intersection, within floating-point
  tolerance (see TEST_PLAN). A BVH that changes the
  image beyond tolerance is a bug, not an optimisation.
- Performance: on a scene with many primitives, BVH
  rendering must be measurably faster than brute force
  (timed, see TEST_PLAN). No speedup number is claimed
  in advance.

## Numbers and determinism

- All math in double precision unless Ch. 3/6 say
  otherwise (verify). Rounding-error handling from
  Ch. 6 is a behaviour to test, not a comment to add.
- Sampling: seeded, deterministic for a given seed, so
  the same invocation produces the same image. The
  seed mechanism and default seed are fixed at code
  time and recorded here.
- Output: format decided at code time (PPM proposed).
  No tone mapping or gamma handling is added unless
  Ch. 5/Ch. 13 require it — if added, it is recorded
  here with its chapter reference.

## Failure modes

- Missing/invalid CLI arguments: usage to stderr, non-zero
  exit, no output file presented as success.
- A scene with no primitives renders background only —
  correct, not an error, and tested as such.
- A write failure (e.g. output directory does not exist):
  error message, non-zero exit, no partial file presented
  as success.
