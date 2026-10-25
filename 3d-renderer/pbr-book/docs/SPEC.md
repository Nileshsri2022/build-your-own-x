# SPEC — PBR Book, Scoped Subset (Ch. 1–7, 9, 13)

## 1. Idea

Render a 3D scene by physically based path tracing: simulate
light transport by tracing rays from the camera into the
scene, letting them bounce between surfaces according to
reflection models, and accumulating the light that reaches
each pixel. This is the book's core idea, restricted to the
scoped chapters in the README. Everything below that is not
marked "per contents page" is a proposal for this build, not
a quotation from the book.

## 2. Scoped chapters and what each contributes

All chapter/section titles below are per the 4th-edition
contents page (fully read). Internals are **per contents
page, to verify against chapter text before code** — the
chapter bodies were not read for this docs set.

- **Ch. 1 Introduction.** Orientation only: literate
  programming, the ray-tracing algorithm, system overview,
  how to proceed through the book, using/understanding the
  code, brief history. Nothing from this chapter is itself a
  feature to implement; it frames the rest.
- **Ch. 2 Monte Carlo Integration.** Basics, improving
  efficiency, sampling using the inversion method,
  transforming between distributions. Per contents page, to
  verify before code: which estimators and sampling
  transforms the simple path tracer in Ch. 13 actually
  relies on, so this chapter is implemented only to that
  depth, not in full generality.
- **Ch. 3 Geometry and Transformations.** Coordinate systems,
  n-tuple base classes, vectors, points, normals, rays,
  bounding boxes, spherical geometry, transformations,
  applying transformations, interactions. This is the
  math foundation: a ray is an origin + direction (+ a
  maximum distance, per contents-page "Interactions" —
  verify), a surface interaction carries position, normal,
  and material data (verify).
- **Ch. 4 Radiometry, Spectra, and Color.** Radiometry,
  working with radiometric integrals, surface reflection,
  light emission, representing spectral distributions,
  color. Per contents page, to verify before code: whether
  this build uses full spectral rendering or a simpler
  RGB approximation, and what the book's simple path
  tracer assumes. Do not assume either in code until that
  check is done; record the decision in DESIGN at that
  point.
- **Ch. 5 Cameras and Film.** Camera interface, projective
  camera models, spherical camera, film and imaging.
  Scope note: the projective camera is the primary
  camera for this build; the spherical camera is included
  in the chapter list but is a stretch goal, not a
  must-have — a build with only the projective camera
  still satisfies this docs set. Film/imaging: the
  mechanism that accumulates pixel samples into an image
  (verify against chapter text).
- **Ch. 6 Shapes.** Basic shape interface, spheres,
  cylinders, disks, triangle meshes, bilinear patches,
  curves, managing rounding error. Scope note: spheres,
  cylinders, disks, and triangle meshes are the must-have
  shapes; bilinear patches and curves are stretch goals.
  Rounding-error handling is a must-have behaviour
  (no cracks / no missed hits at shared edges beyond a
  stated tolerance — see TEST_PLAN), not an optional
  polish item.
- **Ch. 7 Primitives and Intersection Acceleration.**
  Primitive interface and geometric primitives, aggregates,
  bounding volume hierarchies. The BVH is a must-have:
  a scene must render correctly with and without it
  (same image within tolerance — see TEST_PLAN), and
  the BVH must actually accelerate a large scene
  (measured, not asserted).
- **Ch. 9 Reflection Models.** BSDF representation, diffuse
  reflection, specular reflection and transmission,
  conductor BRDF, dielectric BSDF, roughness using
  microfacet theory, rough dielectric BSDF. Excluded
  from this chapter for this build: measured BSDFs and
  scattering from hair. Per contents page, to verify
  before code: the exact BSDF interface (sampling,
  evaluation, PDF) the path tracer in Ch. 13 expects.
- **Ch. 13 Light Transport I: Surface Reflection.** The
  light transport equation, path tracing, a simple path
  tracer, a better path tracer. The **simple path tracer**
  is the target; the "better path tracer" is a stretch
  goal only after the simple one passes its tests.
  Per contents page, to verify before code: recursion
  depth / termination rule, Russian roulette if any,
  and how direct vs indirect light is sampled.

## 3. Dependency note — Ch. 8 (Sampling and Reconstruction)

Ch. 8 is **not** in scope as a chapter to implement in full.
But a path tracer needs sampling, and Ch. 13's simple
tracer may rely on Ch. 8 material (sampling theory,
sampler interface, stratified/Halton/Sobol samplers, image
reconstruction). That dependency must be checked against
the chapter text before code:

- If the simple tracer needs only basic random sampling
  (already covered by Ch. 2's inversion method), no Ch. 8
  work is added.
- If it needs a sampler interface or a specific sampler,
  the minimum sampler needed is added as a scoped
  sub-ticket, and SPEC/DESIGN are updated to say so —
  Ch. 8 as a whole is still not in scope.

Do not silently import Ch. 8.

## 4. Scene and rendering behaviour (proposed, to verify)

- Scene: a set of primitives (shapes + materials), at
  least one camera, at least one light source sufficient
  for the simple path tracer's tests. The exact default
  scene (objects, materials, camera position, light
  positions) is to be fixed at code time from Ch. 13's
  simple tracer examples, verified against the chapter
  text, and written into DESIGN before code. These docs
  deliberately do not invent a default scene and call it
  the book's.
- Rendering loop (proposed): for each pixel, take N
  samples; for each sample, generate a camera ray, trace
  it through the BVH, shade the hit via the BSDF and the
  path-tracing recursion, accumulate; write the averaged
  image to the output file. N, recursion depth, and
  termination are Ch. 13 values, verified before code.
- Colour handling: clamp / tone handling is whatever
  Ch. 5 (Film) and Ch. 13 require — verify, do not invent
  a tone-mapping operator in code without a SPEC note.

## 5. Output format — decision point (see README)

No format is claimed from the book. Proposal: PPM
(plain, pixel-readable, no encoder dependency). The
format is decided at code time and recorded in DESIGN;
TEST_PLAN's pixel checks must be written for the chosen
format before code. If the chosen format needs an
external library, that library is a DESIGN change, not
a silent addition.

## 6. Non-goals (honest)

Volume scattering, GPU wavefront rendering, hair
scattering, measured BSDFs, textures/materials as a
chapter (Ch. 10), full light-source sampling (Ch. 12),
volume rendering (Ch. 14), scene-file parsing
(Appendix C), real-time or GPU rendering, animation,
and any claim to be pbrt. The book's system is the
reference; this build is a scoped subset of it.
