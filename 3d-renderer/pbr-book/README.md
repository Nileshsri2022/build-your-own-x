# 3D Renderer — Physically Based Rendering (PBR Book, Scoped Subset)

Status: **Code built and tested 2026-10-25: C++ scoped subset (path-tracer core), 45 checks + integration PASS.**

## Source link

- "Physically Based Rendering: From Theory To Implementation",
  C++, http://www.pbr-book.org/
- 4th edition contents: https://pbr-book.org/4ed/contents.html
- The book is a textbook building a complete system (pbrt) in a
  literate-programming structure. It is **not** one small project.

## CRITICAL SCOPE RULE — read this first

This build does **NOT** cover the whole book. The whole book is
16 chapters + 3 appendices and is far larger than one BYOX
project. This docs set scopes the build to a **subset only**:

### In scope (the target)

- Ch. 1 Introduction — literate-programming structure, the
  ray-tracing algorithm, system overview (as orientation only)
- Ch. 2 Monte Carlo Integration — basics, improving efficiency,
  inversion method, transforming between distributions
- Ch. 3 Geometry and Transformations — coordinate systems,
  vectors, points, normals, rays, bounding boxes, spherical
  geometry, transformations, interactions
- Ch. 4 Radiometry, Spectra, and Color — radiometry, radiometric
  integrals, surface reflection, light emission, spectral
  distributions, color
- Ch. 5 Cameras and Film — camera interface, projective camera
  models, spherical camera, film and imaging
- Ch. 6 Shapes — basic shape interface, spheres, cylinders,
  disks, triangle meshes, bilinear patches, curves, managing
  rounding error
- Ch. 7 Primitives and Intersection Acceleration — primitive
  interface, aggregates, bounding volume hierarchies (BVH)
- Ch. 9 Reflection Models — BSDF representation, diffuse
  reflection, specular reflection and transmission, conductor
  BRDF, dielectric BSDF, roughness via microfacet theory,
  rough dielectric BSDF
- Ch. 13 Light Transport I: Surface Reflection — the light
  transport equation, path tracing, a simple path tracer
  (the target renderer; the "better path tracer" is a stretch
  goal only if the simple one passes its tests first)

That is: chapters 1–7, plus 9 and 13. Together they form a
path-tracer core: geometry + cameras + shapes + BVH +
reflection models + a simple path tracer.

### Explicitly OUT of scope for this one build

- Ch. 8 Sampling and Reconstruction (as a full chapter — see
  the dependency note in SPEC)
- Ch. 10 Textures and Materials
- Ch. 11 Volume Scattering
- Ch. 12 Light Sources as a full chapter (point/distant/area/
  infinite-area lights and light sampling are not built in
  full; the simple path tracer's lighting is whatever Ch. 13's
  simple tracer requires, verified against the chapter text)
- Ch. 14 Light Transport II: Volume Rendering
- Ch. 15 Wavefront Rendering on GPUs
- Ch. 16 Retrospective and the Future
- Appendices A (Sampling Algorithms), B (Utilities),
  C (Processing the Scene Description)
- Hair scattering (Ch. 9), measured BSDFs (Ch. 9),
  volume scattering (Ch. 11), GPU wavefront rendering
  (Ch. 15) — named here so nobody adds them silently later

If any of the above is wanted, it is a separate follow-on
project, not a hidden extension of this one.

## Honesty about what was read

- The 4th-edition contents page was fully read (all chapters
  and appendices, with section titles). The chapter list in
  SPEC/DESIGN/TICKETS comes from that page.
- Chapter 1's introduction landing page was read (56 lines).
- Core-chapter internals (Ch. 2–7, 9, 13) were **NOT**
  body-read. Anything in these docs that describes a
  chapter's internals is marked "per contents page, to verify
  against chapter text before code". Do not treat those
  descriptions as quoted from the book, and do not copy the
  book's code — this implementation is to be written from
  the concepts, in its own structure.

## Language and build

- Language: **C++**, following the book's language.
- No C++ standard, compiler, or build system is claimed here:
  the pre-code toolchain check in TEST_PLAN (confirm a C++
  compiler is available in the build environment, record its
  version) must pass before any "tested" claim is made.
- No third-party libraries are assumed. Any library the code
  ends up needing must be named in a revised DESIGN before
  code, not discovered silently during coding.

## Output format — NOT established, to be decided at code time

The book's Film/Imaging section (Ch. 5) and Utilities/Images
(Appendix B) exist, but no output format (PNG, EXR, PPM, or
other) was stated on the pages read. So these docs do **not**
claim the book outputs any particular format.

- **Proposal, not the book's format:** use PPM for the first
  build, because it is the simplest pixel-verifiable format
  (plain text header + RGB values, no encoder library needed)
  and TEST_PLAN's pixel checks can read it directly.
- The format decision is a code-time decision, recorded in
  DESIGN at that point. If PPM is rejected, the replacement
  format and its reader must be written into TEST_PLAN
  before code.

## What this build will be (if developed)

A small, offline, physically based path tracer: a scene of
spheres/cylinders/disks/triangle meshes, a projective camera,
BVH-accelerated intersection, BSDF reflection models
(diffuse, specular, conductor, dielectric, microfacet
roughness), and a simple path tracer producing an image
file. No GUI window, no real-time preview, no GPU work.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — scoped chapters, concepts, and every
  divergence / open question
- `docs/DESIGN.md` — module layout, rendering loop, file
  format decision point
- `docs/TEST_PLAN.md` — pixel/behaviour tests, plus the
  toolchain pre-check
- `docs/TICKETS.md` — build order, following the scoped
  chapters, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Out of scope, honestly

Everything listed under "Explicitly OUT of scope" above,
plus: real-time rendering, animation, scene-file parsing
(Appendix C), and any claim that this build is pbrt or a
production renderer. It is a scoped subset build inspired
by the book's chapters, not the book's system.

## Next step

Parent's approval triggers, in order: verify scoped
chapter internals against chapter text, decide the output
format, pass the toolchain check, then code (TDD, one
ticket at a time), test, review against SPEC, then push —
this subfolder only, as its own docs/code set.
