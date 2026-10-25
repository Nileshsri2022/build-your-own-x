# SPEC — 3D Renderer (Avik Das, lessons 0–6)

## 1. Idea

Render a 3D scene to a 2D image by backward ray tracing. Light
in the real world travels from lights to objects to the camera;
most of it never reaches the camera, so simulating it forward is
wasteful. Instead, cast a ray from the camera, through each pixel
of the image plane, into the scene, and work out what that ray
hits and how it is lit. That is the whole method; every lesson
adds one piece of it.

## 2. Lessons (tutorial order)

- **0. Background.** Vectors, and why backward tracing.
- **1. Casting rays.** Camera `c`, image plane corners `x1`
  (top-left), `x2` (top-right), `x3` (bottom-left), `x4`
  (bottom-right). A point `p` on the plane at horizontal fraction
  `alpha in [0,1]` and vertical fraction `beta in [0,1]` is found
  by bilinear interpolation:
  `t = (1-alpha)*x1 + alpha*x2`,
  `b = (1-alpha)*x3 + alpha*x4`,
  `p = (1-beta)*t + beta*b`.
  The ray starts at `p` with direction `d = p - c` (the tutorial
  starts the ray at the plane, not at the camera; points between
  camera and plane are not needed). Pixel `(i,j)` uses the centre
  of its region: `alpha = (i + 0.5) / width`,
  `beta = (j + 0.5) / height`.
- **2. Visibility / spheres.** A ray hits a sphere (centre `C`,
  radius `r`) when the quadratic
  `|o + t*d - C|^2 = r^2` has a real root `t > 0`, where `o` is the
  ray origin. Solve `a = d.d`, `b = 2*d.(o-C)`,
  `c = (o-C).(o-C) - r^2`, discriminant `b^2 - 4*a*c`.
  If the discriminant is negative, no hit. Otherwise the nearest
  positive root is the hit distance; the hit point is `o + t*d`
  and the surface normal is `(hit - C) / r`, normalised. The
  visible object for a pixel is the nearest hit across all
  spheres; if none, the pixel is the background colour.
- **3. Phong shading.** At a hit point, colour =
  `ambient + diffuse + specular`, per colour channel, clamped to
  `[0,1]`:
  - ambient: `material.colour * ambientIntensity` (constant, so
    nothing is pitch black).
  - diffuse: `material.colour * light.colour * max(0, N.L)`,
    where `N` is the unit normal and `L` is the unit vector from
    the hit point to the light.
  - specular: `light.colour * pow(max(0, R.V), shininess)`,
    where `R` is the reflection of `-L` about `N`
    (`R = 2*(N.L)*N - L`) and `V` is the unit vector from the hit
    point to the camera. Summed over all lights.
  Material fields: colour (RGB, each in `[0,1]`), shininess
  (a power, e.g. 50), reflectivity (see lesson 5, 0 = matte).
- **4. Shadows.** For each light, cast a shadow ray from the hit
  point (offset by a small epsilon, e.g. `1e-4`, along `N`, so the
  surface does not shadow itself) toward the light. If any sphere
  is hit at a distance less than the distance to the light, that
  light contributes nothing (no diffuse, no specular) for that
  pixel; ambient still applies. This gives hard shadows.
- **5. Reflections.** If a material's reflectivity `k > 0`, cast a
  reflected ray from the hit point along `R` (reflection of the
  incoming ray direction about `N`), recursively shade it, and
  blend: `colour = (1-k) * localColour + k * reflectedColour`.
  Recursion stops at a fixed depth (default 3, i.e. the primary
  ray plus 3 bounces); beyond that, the reflected contribution is
  the background colour. Reflectivity in `[0,1]`; `k = 0` means
  lesson 3/4 behaviour exactly.
- **6. Anti-aliasing (distributed ray tracing).** Instead of one
  ray through the pixel centre, take `S` samples per pixel
  (default 1, CLI-configurable, e.g. 4), each jittered by a
  deterministic pseudo-random offset inside the pixel region
  (seeded, so renders are reproducible), and average the `S`
  colours. This softens jagged sphere edges. `S` must be a
  positive integer; `S = 1` with the centre offset reproduces
  the pre-lesson-6 image exactly.

## 3. Default scene (ours)

- Camera `c = (0,0,0)`.
- Image plane: `x1 = (-1,1,2)`, `x2 = (1,1,2)`,
  `x3 = (-1,-1,2)`, `x4 = (1,-1,2)` (square, plane at z=2).
- Spheres (centre, radius, colour, shininess, reflectivity):
  - `(0,0,5)`, `1`, red `(1,0,0)`, `50`, `0.0`
  - `(-1.5,0,6)`, `1`, green `(0,1,0)`, `50`, `0.2`
  - `(1.5,0,6)`, `1`, blue `(0,0,1)`, `50`, `0.2`
- One point light at `(-5,5,0)`, white `(1,1,1)`,
  ambient intensity `0.1`.
- Background: black `(0,0,0)`.
- Default output: `400 x 400`, `S = 1` unless the CLI says otherwise.

## 4. Output and CLI

- Output is a PNG, written with `java.awt.image.BufferedImage`
  (`TYPE_INT_RGB`) and `javax.imageio.ImageIO`, no external
  libraries, no window.
- CLI: `Main <output.png> [width height samples]`.
  - `output.png` is required and must end in `.png`.
  - `width`, `height` are positive integers; if omitted, `400 400`.
  - `samples` is a positive integer; if omitted, `1`.
  - Bad arguments print usage to stderr and exit non-zero; no
    partial file is left behind on a bad-argument failure.
- Colour conversion: each channel is clamped to `[0,1]`, then
  `round(channel * 255)` to an 8-bit value.

## 5. Non-goals (honest)

No triangles, meshes, textures, refraction, soft shadows,
global illumination, GPU, animation, or scene-file format. The
scene is code (a `Scene` object built in `Main`); a file format
would be a separate project, not a hidden feature.
