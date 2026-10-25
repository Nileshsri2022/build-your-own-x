# TICKETS — Scratchapixel Introduction to Ray Tracing

Build order follows the tutorial's chapters. Each ticket is
done (code + its tests passing) before the next starts,
except where noted. No ticket starts code for a later
chapter early, and no ticket adds follow-on lesson content.

- **T0 — Toolchain check (pre-code, blocks everything).**
  Confirm a C++ compiler, its version, and the C++ standard
  (C++17 proposed). Record the result. If no compiler is
  available, stop here and report that — do not claim tests
  pass on a machine that cannot compile the code.
  Blocks: T1–T6.
- **T1 — Vectors, rays, camera (chapter 1).** `Vec3`, `Ray`,
  eye origin, image plane, one primary ray per pixel
  centre. No objects yet, or a stub that always misses
  (background only) so the image is checkable.
  Blocks: everything else. Tests: vector tests, a
  background-only render of the requested size.
- **T2 — Spheres and visibility (chapter 2).** `Sphere`
  intersection, nearest-hit selection, background colour,
  a renderer that renders unshaded spheres (hit = material
  colour, miss = background) so the image is checkable
  before shading exists. Blocks: T3–T5.
  Tests: sphere intersection tests, normal test, centre
  pixel = sphere colour, corner = background.
- **T3 — Basic shading and shadows (chapter 2).** Object
  colour × light brightness, shadow rays per light,
  epsilon offset, blocked-light test. Blocks: T4.
  Tests: shadow tests (lit vs blocked), shadowed pixel
  black.
- **T4 — Reflection and refraction (chapter 3).** Recursive
  `trace` with a depth limit, reflection vector, refraction
  for glass, Fresnel / facing-ratio blend. This ticket
  includes the "to verify before code" step for the exact
  Fresnel formula and refractive index — those must be
  verified from chapter 3 and recorded before this
  ticket's code is accepted. Blocks: T5.
  Tests: reflection/refraction blend tests, depth-limit
  termination test, glass-sphere shadow limitation noted
  (transparent sphere still casts a solid shadow, per
  SPEC).
- **T5 — PPM output, CLI, and integration (chapters 1–3).**
  `PpmWriter`, CLI argument validation, integration
  script (exit codes, PPM validity, centre/corner pixels).
  Blocks: T6. Tests: PPM writer tests, integration test.
- **T6 — Code review + push prep.** Review against SPEC,
  fix the camera/image-plane numbers and `MAX_DEPTH`
  in the code and record them, check no out-of-scope
  claims, run the full suite + a default-scene render
  for eyeballing, then present for approval. Push itself
  is separate, on approval, and for this folder only.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6, strictly linear. There is
no parallel ticket here: each chapter's behaviour is the
foundation for the next, and a parallel build would hide a
regression in an earlier chapter behind a later one.
