# SPEC — Scratchapixel Introduction to Ray Tracing

## 1. Idea

An image is a cut through a pyramid whose apex is at the eye
and whose height runs along the line of sight. The cut surface
is the **image plane** — the canvas onto which the 3D scene is
projected. Rendering, in this tutorial, is two steps: project
the scene's outlines onto that plane (geometry), then colour
inside the outlines (shading).

Backward ray tracing avoids simulating light that never reaches
the eye. Instead of tracing rays from lights outward, cast a ray
from the eye, through each pixel of the image plane, into the
scene, and work out what it hits and how that point is lit.
That is the whole method in the read chapters; every chapter
adds one piece of it.

## 2. Chapters (tutorial order, as read)

- **1. How Does it Work.** Perspective projection: a line from
  each object point to the eye marks the image plane where it
  crosses, and joining those marks draws the object's outline.
  Light and colour: photons are absorbed, reflected, or
  transmitted by a material, and the counts must add up
  (energy conservation). Materials are broadly conductors
  (metals) or dielectrics (non-metals: glass, plastic, wood,
  water), differing in transparency. For opaque diffuse
  materials, an object's colour under white light is which
  wavelengths it reflects rather than absorbs — a red object
  reflects red photons and absorbs blue and green.
- **2. Implementing the Raytracing Algorithm.** One primary ray
  per pixel, through the centre of that pixel's region of the
  image plane. Test the ray against every object; keep the
  nearest intersection. At the hit point, cast a shadow ray
  toward each light: if anything blocks it, the point is in
  shadow for that light. Colour the pixel from the hit object;
  if nothing is hit, colour it with the background.
- **3. Writing a Basic Raytracer.** A working example program
  and recursive-trace pseudocode, in a few hundred lines.
  Spheres only. The trace function is recursive: a hit may
  spawn reflected and/or refracted rays, whose colours feed
  back into the hit pixel's colour. A visible light-source
  sphere is part of the example scene, so a ray that hits it
  returns the light's colour directly.

## 3. Geometry and math

- **Ray.** An origin point plus a direction vector; a point on
  the ray is `origin + t * direction` for `t >= 0`. Primary
  rays start at the eye and pass through pixel centres.
- **Sphere intersection.** A ray hits a sphere (centre `C`,
  radius `r`) when `|o + t*d - C|^2 = r^2` has a real root
  `t > 0`, where `o` is the ray origin and `d` its direction.
  Solve `a = d.d`, `b = 2*d.(o-C)`,
  `c = (o-C).(o-C) - r^2`, discriminant `b^2 - 4*a*c`.
  Negative discriminant: no hit. Otherwise the nearest
  positive root is the hit distance; the hit point is
  `o + t*d` and the surface normal is `(hit - C) / r`,
  normalised. The visible object for a pixel is the nearest
  hit across all spheres.
- **Shadow test.** From the hit point, cast a ray toward the
  light. If any sphere is hit at a distance less than the
  distance to the light, that light is blocked for this
  pixel. (An epsilon offset along the normal, e.g. `1e-4`,
  is our implementation detail to avoid the surface
  shadowing itself; the tutorial's chapters do not prescribe
  a value — see DESIGN.)
- **Reflection.** Reflect the incoming direction about the
  surface normal: `R = d - 2*(d.N)*N` for unit `d` and `N`
  (equivalently, the reflection of the reversed incoming ray).
  Trace `R` recursively and mix its colour in by the
  material's reflectivity.
- **Refraction and Fresnel (chapter 3 only).** A glass sphere
  splits light into a reflected ray and a refracted ray.
  Which share dominates is decided by a Fresnel / facing-ratio
  term: rays hitting the surface head-on refract more, rays
  at a grazing angle reflect more. The exact Fresnel formula
  and refractive index values are **to verify before code**
  from the chapter-3 text — these docs do not state a
  formula that was not verified in the read material.
  Recursion must be depth-limited (a fixed maximum depth,
  e.g. 5, to be fixed in code review) so a pair of facing
  reflective spheres cannot recurse forever.
- **Transparent-sphere shadow, honestly noted.** The tutorial
  itself flags that a transparent sphere casting a solid
  shadow is unrealistic. This build reproduces that behaviour
  in the read-chapters scope (shadow rays block light
  regardless of material) and documents it as a known
  limitation, rather than silently "fixing" it with
  light-through-glass handling that the read chapters do
  not teach.

## 4. Shading (read chapters only — basic, not Phong)

- Hit pixel colour = object colour × light brightness,
  summed over unblocked lights, clamped to `[0,1]` per
  channel.
- Shadowed for a light = that light contributes 0 (not a
  dimmed value); if all lights are blocked, the pixel is
  black (background colour is black unless the scene says
  otherwise).
- No diffuse/specular split, no shininess, no ambient term
  is spec'd here, because the read chapters do not teach
  one. Adding Phong would be a different tutorial's scope,
  not this build's.
- Reflection/refraction (chapter 3 only) are blended into
  the hit colour by the Fresnel term above; an opaque,
  non-reflective sphere uses the basic shading alone.

## 5. Default scene (ours)

- Eye/camera at the origin `(0,0,0)`, looking along the
  forward axis toward the image plane (exact plane
  placement and field of view to be fixed in DESIGN and
  confirmed in code review — the tutorial's example
  geometry values were not all verified from the text
  fetch, so they are not claimed here).
- Spheres (centre, radius, colour, material):
  - one ordinary opaque sphere, red
  - one reflective sphere, green
  - one glass sphere, blue
  - one small visible light-source sphere, white
- One point light (position to be fixed in DESIGN; the
  visible light-source sphere is the light's visible
  marker, not a second, separate light unless DESIGN
  says so).
- Background: black `(0,0,0)`.
- Default output: a small image for testing (e.g.
  `200 x 200` in TEST_PLAN) and a full-size render only
  for eyeballing; the tutorial's examples use PPM files
  such as `untitled.ppm` / `minray.ppm`.

## 6. Output and CLI

- Output is a PPM file (ASCII `P3` in the tutorial's
  examples), written with the C++ standard library only
  (no image library, no GUI).
- CLI (proposed, to be confirmed at code time): the program
  takes an output path, and optionally width and height.
  Bad arguments print usage to stderr and exit non-zero;
  no partial file is presented as a success.
- Colour conversion: each channel is clamped to `[0,1]`,
  then `round(channel * 255)` to an 8-bit value, written
  as decimal integers in the PPM body, with the header
  `P3`, width, height, and max value `255`.

## 7. Non-goals (honest)

No Phong/diffuse/specular shading, no textures, no
anti-aliasing, no triangle meshes (no Utah Teapot), no area
lights, no path tracing, no motion blur, no depth of field,
no GUI, and no follow-on lesson content. The scene is code
(a list of spheres built in `main`); a scene-file format
would be a separate project, not a hidden feature.
