# DESIGN — 3D Renderer

## Layout

```
pom.xml
src/main/java/com/renderer/
  Vec3.java          # immutable 3D vector: add, sub, scale, dot, cross, length, normalize, lerp
  Ray.java           # origin + direction, pointAt(t)
  Camera.java        # camera + image-plane corners, rayForPixel(i, j, width, height, jitter)
  Sphere.java        # centre, radius, material; intersect(ray) -> Hit or null
  Material.java      # colour, shininess, reflectivity
  Light.java         # position, colour
  Scene.java         # spheres, lights, background, ambientIntensity, defaultScene()
  Color.java         # RGB in [0,1]: add, scale, multiply, clamp, toRgbInt()
  Renderer.java      # render(scene, camera, width, height, samples, seed) -> BufferedImage
  PngWriter.java     # write BufferedImage to a .png path (thin ImageIO wrapper, for tests)
  Main.java          # CLI parsing, builds default scene, renders, writes PNG
src/test/java/com/renderer/
  Vec3Test.java, SphereTest.java, ShadingTest.java, RendererTest.java
```

Package is `com.renderer`, not the tutorial repo's package (which
was not read for these docs); class names follow the concepts in
SPEC, which is what matters for readability.

## Rendering loop

```
for each pixel (i, j):
  colour = black
  for each of S samples:
    (alpha, beta) = pixel centre + jitter offset (seeded Random, one
      stream per render, in row-major pixel order, so output is
      deterministic for a given seed)
    ray = camera.rayForPixel(alpha, beta)
    colour += trace(ray, depth = 0)
  image[i][j] = colour / S
```

`trace(ray, depth)`:

```
hit = nearest sphere intersection with t > EPSILON (1e-4)
if hit is null: return background
local = phong(hit, scene)            # ambient + diffuse + specular, with shadow test per light
if material.reflectivity == 0 or depth >= MAX_DEPTH (3): return local
reflectedRay = Ray(hit.point + normal * EPSILON, reflect(ray.direction, normal))
reflected = trace(reflectedRay, depth + 1)
return (1 - k) * local + k * reflected
```

`phong(hit, scene)`:

```
colour = material.colour * ambientIntensity
for each light:
  L = normalize(light.position - hit.point)
  shadowOrigin = hit.point + normal * EPSILON
  if any sphere is hit by Ray(shadowOrigin, L) with t < distance to light:
    continue   # light blocked, ambient only for this light
  colour += material.colour * light.colour * max(0, N.L)
  R = 2 * (N.L) * N - L
  V = normalize(camera - hit.point)
  colour += light.colour * pow(max(0, R.V), shininess)
return colour.clamped()
```

Note: the camera position for `V` is passed into `trace` via the
scene/camera (Renderer holds the camera), not recomputed per hit.

## Numbers and determinism

- All math in `double`. Epsilon `1e-4` for self-intersection and
  shadow origins, and for rejecting hits behind / at the ray
  origin (`t > EPSILON`, not `t > 0`).
- Jitter: `Random(seed)` with a fixed default seed (e.g. `42`),
  so the same CLI invocation always produces the same PNG
  (byte-identical, since PNG encoding of the same pixels is
  deterministic). Tests must not depend on a particular Random
  implementation's sequence beyond "same seed → same image";
  pixel tests use `S = 1` (no jitter) wherever an exact colour
  is asserted.
- PNG: `BufferedImage.TYPE_INT_RGB`, `ImageIO.write(image, "png", file)`.
  No gamma correction, no tone mapping — the tutorial does none,
  and adding it would silently change every expected pixel.

## Failure modes

- Unknown output extension, non-positive width/height/samples,
  or missing output argument: usage + exit 1, no file written.
- A write failure (e.g. directory does not exist): the exception
  message is printed and the process exits non-zero; a partially
  written PNG, if any, is not presented as a success.
- A scene with no spheres renders all background — that is
  correct, not an error, and is tested as such.
