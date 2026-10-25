# TICKETS — Ray Tracing in One Weekend (Book 1)

Build order follows the book's chapters. Each ticket is
done (code + its tests passing) before the next starts,
except where noted. Chapters marked "per TOC, to verify
before code" in SPEC must be re-read against the book
text at the start of their ticket — do not code them
from the TOC summary alone.

- **T0 — Toolchain check (before all code).** Confirm
  a C++ compiler in this VM (version, hello-world
  compile + run) and record the build command. Blocks:
  everything. If no compiler is available, code is
  still written, but marked untested-in-this-VM.
- **T1 — Output and vectors (Chapters 2–3).** PPM
  writing (header, triples, no progress text on
  stdout) and the vec3 type with colour utilities.
  Blocks: everything. Tests: vec3 unit tests, PPM
  structure test with a gradient image.
- **T2 — Rays, camera, background (Chapter 4).**
  Ray type, simple camera, sky-gradient background.
  Blocks: T3–T8. Tests: ray `at(t)`, empty-world
  sky-gradient test (top blue, bottom white).
- **T3 — Spheres and hittables (Chapters 5–6).**
  Sphere intersection, hittable abstraction, hit
  record with front-face normal handling, hittable
  list with nearest-hit selection, interval type.
  Blocks: T4–T8. Tests: sphere hit/miss/inside/
  nearest tests, single-sphere centre/corner test.
- **T4 — Camera class (Chapter 7).** Move camera
  code into its own type owning aspect, width,
  samples, depth, and the render loop.
  Blocks: T5–T8. Tests: PPM dimensions at 16:9 and
  square, render loop unchanged from T3 images
  within tolerance.
- **T5 — Antialiasing (Chapter 8).** Random utilities,
  jittered multi-sample per pixel, sample-square ray
  construction, averaged colour writing with clamp
  and scale. Blocks: T6–T8. Tests: AA edge-softening
  test (1 vs many samples), interior unchanged
  within tolerance.
- **T6 — Diffuse materials (Chapter 9).** Lambertian
  scatter, recursion with depth limit, shadow-acne
  interval fix (0.001), gamma correction (verify
  against Chapter 9 before code). Blocks: T7–T8.
  Tests: Lambertian centre-vs-limb brightness,
  no-black-speckle contact test, no-cast-shadow
  test, depth-limit termination test.
- **T7 — Metal and dielectrics (Chapters 10–11).**
  Abstract material, metal (mirrored + fuzzy),
  dielectric (refraction, total internal reflection,
  Schlick, hollow glass sphere). Verify both chapters
  against the book text first — they are per-TOC
  in SPEC. Blocks: T8. Tests: metal reflection
  test, fuzzy-vs-sharp test, dielectric
  not-flat-colour test, hollow-glass renders.
- **T8 — Positionable camera, defocus, final render
  (Chapters 12–14).** Camera position/orientation,
  thin-lens defocus blur, final scene (ground +
  three feature spheres + random small spheres).
  Verify Chapters 12–13 against the book text first
  — they are per-TOC in SPEC. Blocks: T9.
  Tests: final-render integration test (exit 0,
  complete PPM, centre = feature sphere family,
  corners = sky), defocus blur softens out-of-focus
  edges beyond tolerance vs in-focus render.
- **T9 — Code review + push prep.** Review against
  SPEC, check no out-of-scope claims (no Phong, no
  lights, no shadows, no meshes/textures), run the
  full suite + a final render for eyeballing, then
  present for per-push approval. Push itself is
  separate, on approval, and covers this folder
  only (as part of the 3d-renderer docs/code push).

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6 → T7 → T8 → T9,
strictly linear. Each chapter's behaviour is the
foundation for the next, and a parallel build would
hide a regression in an earlier chapter behind a
later one.
