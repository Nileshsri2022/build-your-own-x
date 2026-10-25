# SPEC — Ray Tracing in One Weekend (Book 1)

## 1. Idea

Render a scene by tracing rays backwards from the camera.
For each pixel, cast one or more rays into the scene, find
the nearest object each ray hits, and compute a colour by
bouncing that ray according to the material it hit, until
the ray escapes to the background or a recursion limit is
reached. Averaging many jittered samples per pixel gives
anti-aliasing. The book calls the result a path tracer,
because colour comes from randomly scattered bounce rays
rather than from direct lights.

There are no explicit lights in Book 1. Brightness comes
from the sky-gradient background and from how materials
scatter (attenuate) that background light on each bounce.

## 2. Chapters (book order)

- **1. Overview — per TOC, to verify before code.** The
  book presents itself as a weekend build of a path tracer
  in C++, using no graphics API. No internals are claimed
  here beyond that framing.
- **2. Output an Image — per TOC, to verify before code.**
  PPM image format, creating an image file, progress
  indicator. Output established from the rendering code
  seen in Chapter 8: ASCII PPM, header `P3`, then width,
  height, then `255`, then one RGB triple per pixel.
- **3. The vec3 Class — per TOC, to verify before code.**
  A 3-component vector type used for both points and
  colours, with utility functions for colour handling.
- **4. Rays, a Simple Camera, and Background — per TOC,
  to verify before code.** A ray type (origin + direction),
  a simple camera sending rays into the scene, and a
  background colour function.
- **5. Adding a Sphere — per TOC, to verify before code.**
  Ray–sphere intersection: solve for the parameter `t`
  where the ray meets the sphere surface, using the
  quadratic form of the ray–sphere equation; the smallest
  positive root in the accepted interval is the hit.
  First raytraced image: a single sphere against the sky.
- **6. Surface Normals and Multiple Objects — per TOC,
  to verify before code.** Shading with surface normals;
  a hittable abstraction so a scene is a list of hittable
  objects; front faces versus back faces (a hit is
  classified by whether the ray hits the outward-facing
  side, and the normal is flipped to face the ray when
  needed); common constants and utilities; an interval
  type for accepted hit ranges.
- **7. Moving Camera Code Into Its Own Class — per TOC,
  to verify before code.** The camera becomes its own
  type, owning image size, aspect, samples, and the render
  loop, rather than living in the main program.
- **8. Antialiasing — body read (tail of chapter).**
  Random utilities first, then multiple samples per
  pixel: for each pixel, take `samples_per_pixel` rays,
  each through a randomly jittered point inside the
  pixel square (an offset in [-0.5, +0.5] in each axis
  from the pixel centre), sum their colours, and divide
  by the sample count. Colour writing clamps each
  channel to [0, 1) (interval clamp, upper bound
  exclusive in the code seen) and scales by 256 to a
  byte. Camera parameters seen: aspect ratio 16:9,
  image width 400 in the main program, 10 samples per
  pixel in the camera listing, 100 in the main program.
  The camera's ray for pixel (i, j) is built from a
  precomputed first-pixel location plus per-pixel
  horizontal and vertical steps, with the jittered
  offset applied.
- **9. Diffuse Materials — body read (most of chapter).**
  A Lambertian (matte) material: on a hit, scatter a new
  ray whose direction is the surface normal plus a
  random unit vector, and attenuate the colour by the
  material's albedo. Recursion: each bounce spawns a
  child ray; a depth limit stops the chain (10 in the
  listings seen, 50 in the main program seen), and a ray
  that exhausts its depth returns black. Shadow acne:
  hits are only accepted from `t = 0.001` upward, so a
  scattered ray does not immediately re-hit its own
  surface — this is a numerical fix, NOT a shadow
  feature, and must not be described as shadows.
  Gamma correction is listed in this chapter's TOC
  (per TOC, to verify before code): colour values are
  gamma-corrected when written out, so that linear
  light values display correctly.
- **10. Metal — per TOC, to verify before code.**
  An abstract material type; an intersection record
  describing a hit; mirrored reflection for metal;
  a scene with metal spheres; fuzzy reflection
  (a small random perturbation of the reflected
  direction, giving a blurred mirror).
- **11. Dielectrics — per TOC, to verify before code.**
  Refraction via Snell's law; total internal
  reflection when a ray inside glass cannot escape;
  the Schlick approximation for the probability of
  reflection versus refraction at a glass surface;
  a hollow glass sphere (two concentric spheres with
  opposite face orientation) as the demonstration.
- **12. Positionable Camera — per TOC, to verify before
  code.** Camera viewing geometry: position and
  orientation of the camera, so the camera is no longer
  fixed at the origin looking down one axis.
- **13. Defocus Blur — per TOC, to verify before code.**
  A thin-lens camera approximation: rays originate from
  random points on a lens disk rather than a single
  point, so objects away from the focus distance blur.
- **14. Where Next? — per TOC, to verify before code.**
  A final render, then pointers onward: Book 2
  (Ray Tracing: The Next Week) and Book 3 (Ray Tracing:
  The Rest of Your Life), which are out of scope here.

## 3. Scene (Book 1 final render, per TOC/headings)

- A large ground sphere (Lambertian) as the floor.
- Three large feature spheres in the centre: one
  Lambertian, one metal, one dielectric (glass).
- Many small random spheres scattered on the ground,
  each with a randomly chosen material type among
  Lambertian, metal, and dielectric.
- Background: a vertical sky gradient, white at the
  bottom blending to blue `(0.5, 0.7, 1.0)` at the top
  in the listings seen; the exact blend is a linear
  interpolation by the ray direction's normalised
  y-component, per the background function described
  in the book's early chapters (per TOC, to verify
  before code — the two endpoint colours above are
  from the code seen, the interpolation detail is
  to be confirmed against Chapter 4 before code).
- No point lights, no directional lights, no shadow
  rays, no explicit light list. Do not add any.

## 4. Materials, precisely

- **Lambertian (diffuse):** scatter direction = normal
  + random unit vector; colour attenuated by albedo.
  If the scattered direction is near zero (degenerate),
  fall back to the normal itself — this guard is a
  standard part of the book's diffuse material and
  is to be confirmed against Chapter 9 before code.
- **Metal:** reflect the incoming direction about the
  normal; if the reflected ray goes below the surface
  (dot with normal <= 0), the ray is absorbed
  (returns black). Fuzzy metal adds a random vector
  scaled by a fuzz factor to the reflected direction.
  Attenuation = metal colour (albedo).
- **Dielectric (glass):** refractive index (1.5 for
  glass in the book's usage, per TOC, to verify
  before code). On a hit, choose reflection or
  refraction: refract unless total internal
  reflection applies or the Schlick reflectance
  probability, sampled randomly, says reflect.
  Attenuation is white (no colour loss) in the book's
  glass material, per TOC, to verify before code.

## 5. Output and run

- Output is ASCII PPM (`P3`) on stdout: header lines
  `P3`, `<width> <height>`, `255`, then RGB integer
  triples, one per pixel, row-major from the top row.
- The program takes no scene file and no command-line
  scene arguments in Book 1 as described; scene and
  camera parameters are constants in the main program.
  If a later ticket adds CLI arguments (width,
  samples), that is a deliberate extension and must
  be documented as such, not presented as the book's.
- Colour conversion, per the code seen in Chapter 8:
  average samples, apply gamma correction (per
  Chapter 9 TOC, to verify before code), clamp each
  channel to [0, 1), multiply by 256, truncate to an
  integer in [0, 255].

## 6. Non-goals (honest)

No Phong shading, no explicit lights, no shadow rays,
no triangles, meshes, textures, motion blur, or
bounding-volume hierarchies in Book 1. Those belong to
Books 2–3 or to other tutorials, and are not claimed.
No PNG/TGA output, no window, no GPU use.
