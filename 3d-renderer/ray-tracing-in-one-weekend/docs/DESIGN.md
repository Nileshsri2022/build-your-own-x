# DESIGN — Ray Tracing in One Weekend (Book 1)

## Layout

The file layout follows the book's own organisation
(paraphrased, not copied). Header-only C++ types are the
book's style; whether each type lives in its own header
or is grouped is a code-time decision, recorded here as
the planned layout, not as the book's text.

```
main.cc
vec3.h        # 3-component vector: add, sub, scale, dot, cross,
              #   length, unit vector, random helpers
color.h       # colour writing: gamma correction, clamp, PPM output
ray.h         # origin + direction, point at parameter t
interval.h    # accepted hit range [min, max], contains/surrounds/clamp
hittable.h    # abstract hittable + hit record (point, normal,
              #   t, front face, material pointer)
hittable_list.h  # a list of hittables; nearest hit wins
sphere.h      # centre, radius, material; ray–sphere intersection
material.h    # abstract material + Lambertian, metal, dielectric
camera.h      # image size, aspect, samples, max depth, focus/defocus;
              #   render loop, per-pixel ray construction
```

No build system is prescribed by these docs beyond a
C++ compiler invocation. The exact build command is a
pre-code decision, after the toolchain check (see
TEST_PLAN). No external libraries are needed: standard
library only.

## Core types and their contracts

- **vec3 / point3 / color:** the same 3-double type
  serves as point, direction, and colour. Operations:
  component access, addition, subtraction, scalar
  multiplication/division, dot product, cross product,
  squared length, length, unit vector. Random helpers:
  random double in [0, 1), random double in a range,
  random vector, random unit vector, random vector on
  a hemisphere (per Chapter 9, to verify before code).
- **ray:** origin and direction. `at(t)` returns
  origin + t * direction. Direction need not be unit
  length at construction; callers normalise where the
  math needs it (e.g. background blend, sphere hit).
- **interval:** a [min, max] range with `contains`,
  `surrounds` (strictly inside), and `clamp` operations.
  Hit acceptance uses an interval: the book's ray
  colour function accepts hits in [0.001, infinity)
  to avoid shadow acne (see SPEC §2, Chapter 9).
- **hit_record:** hit point, surface normal, hit
  parameter `t`, a front-face flag, and a pointer to
  the material hit. The normal is set to face against
  the incoming ray: if the ray hits the front face,
  the outward normal is kept; otherwise it is negated.
  This is what makes a hollow glass sphere work:
  the inner sphere's faces are back faces.
- **hittable (abstract):** one method, `hit(ray,
  interval, record) -> bool`. Returns true and fills
  the record for the nearest hit inside the interval.
- **hittable_list:** iterates its objects, keeps the
  closest hit found so far, and shrinks the accepted
  interval's upper bound to that hit's `t` as it goes,
  so a farther object cannot overwrite a nearer one.
- **sphere:** centre, radius, material. Intersection
  solves the ray–sphere quadratic; the discriminant
  decides hit vs miss; the nearest root inside the
  interval is chosen. If neither root is inside, miss.
- **material (abstract):** one method, `scatter(ray,
  record, attenuation, scattered) -> bool`. Returns
  false when the ray is absorbed (metal reflecting
  below the surface is the Book 1 case). Lambertian,
  metal, and dielectric implement it as in SPEC §4.
  Dielectric needs the refractive index and the
  Schlick reflectance function; its scatter chooses
  reflect vs refract per hit, using a random sample.
- **camera:** owns aspect ratio, image width (height
  derived, at least 1), samples per pixel, max depth,
  and, from Chapters 12–13, position/orientation and
  defocus parameters (focus distance, defocus angle,
  lens-disk sampling — per TOC, to verify before
  code). `render(world)` writes the PPM header, then
  for each pixel (top row first) sums
  `samples_per_pixel` ray colours and writes the
  averaged, gamma-corrected, clamped colour.
  Per-pixel ray construction: first-pixel location
  plus (i + jitter) * horizontal step plus
  (j + jitter) * vertical step, where jitter is a
  random offset in [-0.5, +0.5] per axis per sample
  (the sample-square method seen in Chapter 8).

## Rendering loop (ray colour, recursive)

```
color ray_color(ray, depth, world):
  if depth <= 0: return black
  if world.hit(ray, interval(0.001, infinity), record):
      if record.material.scatter(ray, record, attenuation, scattered):
          return attenuation * ray_color(scattered, depth - 1, world)
      else:
          return black
  else:
      # background: sky gradient
      t = 0.5 * (unit(ray.direction).y + 1)
      return (1 - t) * white + t * blue(0.5, 0.7, 1.0)
```

Notes on this pseudocode: it is our paraphrase of the
book's algorithm, not its code. The depth limit is 10
in the early listings seen and 50 in the main program
seen — the code-time decision must pick one and record
it; the final render in the book uses the main-program
value (per TOC, to verify before code). The background
blend detail (which component, which endpoints) is
likewise to be confirmed against Chapter 4 before code.

## Randomness and determinism

- Random numbers come from the standard library
  (`std::mt19937` with a uniform real distribution in
  the code seen in Chapter 8). The book does not
  seed deterministically in the listings seen, so two
  runs of the same program will generally differ in
  fine noise, though the scene and average colours
  are the same.
- TEST_PLAN therefore does NOT assert byte-identical
  PPMs across runs. It asserts dimensions, structural
  pixels (corners = sky, centre = feature sphere),
  material colour families, and edge softening with
  tolerance, and it asserts determinism only for a
  seeded variant if one is added as a deliberate
  test-only extension (recorded as such, not as the
  book's behaviour).

## Failure modes

- No scene file is read, so there is no scene-parse
  failure mode in Book 1.
- A degenerate material or a zero-length scattered
  direction must not produce NaN colours: the
  Lambertian near-zero guard (SPEC §4) and the metal
  below-surface absorption are the book's guards;
  both are to be confirmed against Chapters 9–10
  before code and tested (see TEST_PLAN).
- Output is stdout: a broken pipe or a closed stdout
  is a runtime error of the shell, not a renderer
  bug, and is not tested as one.
