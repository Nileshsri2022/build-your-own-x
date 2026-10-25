# DESIGN — Computer Graphics from Scratch (Gambetta)

## Layout (proposed, not the book's file layout)

The book presents pseudocode plus per-algorithm JavaScript
demos; it does not prescribe one project layout, so the
layout below is ours. It keeps the two renderers separate
except for the shared canvas and math core, so either
sub-build can be developed, tested, and reviewed alone.

```
index.html                  # demo pages: one page per algorithm, as in the book
src/
  canvas.js                 # Canvas wrapper: width/height, PutPixel, pixel readout for tests
  color.js                  # RGB colour: add, scale, multiply, clamp
  math.js                   # Vec3 (add, sub, scale, dot, length, normalize),
                            # Mat4 (multiply, transform point/vector) — see Appendix note below
  raytracer/
    ray.js                  # Ray: origin + direction
    sphere.js               # Sphere: centre, radius, colour, specular exponent, reflectivity
    light.js                # Light: type (ambient/point/directional), intensity, position/direction
    scene.js                # Raytracer scene: spheres, lights, background
    trace.js                # TraceRay: nearest hit, shading, shadow test, reflection recursion
    render.js               # Render loop: one ray per pixel, write via PutPixel
  rasterizer/
    model.js                # Model: vertex list + triangle list (cube as the default)
    instance.js             # Instance: model + transform (scale/rotation/translation)
    camera.js               # Camera: position, orientation, projection parameters
    clip.js                 # Clipping: instance-level and triangle-level
    raster.js               # Triangle filling: lines, filled, shaded, depth buffer, culling
    shade.js                # Flat / Gouraud / Phong shading for triangles
    texture.js              # Textures: UV mapping, perspective-correct interpolation
    render.js               # Render loop: transforms, projection, clipping, rasterize
tests/
  ...                       # see TEST_PLAN.md; one test file per module area
```

Module names follow the concepts in SPEC, not the book's
demo variable names (which were not read in full and are
not reproduced here).

## Shared core

- `canvas.js` owns the only drawing operation: a function
  that sets one pixel. Both renderers call it and nothing
  else to draw. It also owns pixel readout for tests:
  given (x, y), return the RGB value last written there.
  In a browser this maps to a Canvas 2D context; in a test
  harness it maps to an off-screen canvas or an in-memory
  pixel buffer with the same interface. The interface must
  be identical in both cases, or pixel tests prove nothing
  about the demo code.
- `color.js` owns colour arithmetic and clamping. Colours
  are clamped to their valid range before being written;
  out-of-range values must never reach `PutPixel` silently.
- `math.js` owns vectors and matrices. The appendix scope
  is: points, vectors, matrices, and the operations listed
  in SPEC section 2. No other linear-algebra machinery is
  assumed. Matrix layout (row-major vs. column-major) and
  the handedness of the coordinate system must be fixed
  here, in one place, before any renderer code, and both
  renderers must use the same convention. This is the most
  likely source of a "looks right in one renderer, mirrored
  in the other" bug, so it is a design decision, not an
  accident to discover later.

## Sub-build A — Raytracer rendering loop

```
for each pixel (x, y):
  direction = camera ray direction through (x, y)
  colour = TraceRay(camera position, direction, recursion_depth)
  PutPixel(x, y, colour)
```

`TraceRay(origin, direction, depth)`:

```
hit = nearest sphere intersection with t in a valid range
      (t must be positive and, for shadow/reflection rays,
      bounded as SPEC requires)
if hit is null: return background colour
colour = shading(hit)          # ambient + diffuse + specular, per light
if material reflectivity == 0 or depth <= 0: return colour
reflected = TraceRay(hit point, reflected direction, depth - 1)
return colour * (1 - reflectivity) + reflected * reflectivity
```

Shading, per light:

```
if light is ambient: colour += material colour * ambient intensity
if light is point or directional:
  if shadow test finds an occluder between hit point and light:
    contribute nothing for this light
  else:
    colour += material colour * diffuse term (normal vs. light direction)
    colour += specular term (view vs. reflected light direction, ^ exponent)
```

Shadow-test details that must be fixed before code (they
are demo-excerpt details, to verify against chapter 4):
the exact `t` range used for the shadow ray, and the
epsilon offset (if any) used so a surface does not shadow
itself. Do not guess these in code and leave them
undocumented; write them into DESIGN.md as a revision
once verified.

## Sub-build B — Rasterizer rendering loop

```
for each instance in scene:
  transform model vertices by instance transform, then camera transform
  clip instance against the view volume (instance-level)
  for each triangle:
    project its vertices to screen space (perspective)
    clip triangle against the view volume (triangle-level)
    if back-face culling is on and triangle faces away: skip
    rasterize triangle:
      determine covered pixels
      for each pixel:
        if depth buffer says a nearer surface already covers it: skip
        otherwise: shade pixel (flat / Gouraud / Phong, as selected),
                   apply texture if selected (perspective-correct),
                   write pixel, update depth buffer
```

Key design points, each to be verified against its chapter
before code:

- **Depth buffer.** One depth value per pixel, initialised
  to "infinitely far". A pixel is written only when the new
  surface is nearer than the stored depth. The depth value
  stored must be defined (screen-space depth vs. reciprocal
  depth) in one place, because texture interpolation uses
  reciprocal depth and the two must not be confused.
- **Back-face culling.** A toggle, not a hard-coded
  behaviour: the book's demos let the reader switch it on
  and off, and tests must cover both states (see
  TEST_PLAN.md).
- **Shading variants.** Flat, Gouraud, and Phong are three
  modes of the same pipeline, selected per render, not
  three separate renderers. Which normal each mode uses
  (triangle normal vs. per-vertex normals) must be verified
  against chapter 13 before code and recorded here.
- **Textures.** UV coordinates travel with vertices and
  are interpolated across the triangle. Perspective-correct
  interpolation (using reciprocal depth) is the required
  behaviour; linear screen-space interpolation is a
  demonstrably wrong mode to be tested against, not an
  acceptable substitute (see TEST_PLAN.md).

## Failure modes

- A raytracer scene with no spheres renders all background.
  That is correct, not an error.
- A rasterizer scene with a cube entirely behind the camera
  renders all background after clipping. That is correct,
  not an error, and is a required test case.
- An unknown shading mode, an unknown light type, or a
  triangle with degenerate (zero-area) geometry must fail
  loudly in tests, not render a silently wrong image.
  Runtime behaviour for degenerate input (skip vs. throw)
  must be chosen and documented before code; it must not
  differ between the demo page and the test harness.
