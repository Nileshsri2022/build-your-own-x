# TICKETS — PBR Book, Scoped Subset

Build order follows the scoped chapters. Each ticket is
done (code + its tests passing) before the next starts,
except where noted. No ticket starts code for a later
chapter early, and no out-of-scope chapter is started
at any point in this ticket list.

- **T0 — Pre-code checks (blocks everything).** Verify
  scoped chapter internals against chapter text for
  Ch. 2–7, 9, 13 (record verified values in DESIGN:
  BSDF interface, path-tracer depth/termination, miss
  colour, direct-light handling). Decide and record the
  output format (PPM proposed) and the colour model
  (spectral vs RGB). Pass the toolchain check in
  TEST_PLAN §0 (compiler name/version, build method,
  test method). If any check fails, report the blocker;
  do not start T1 on assumptions.
- **T1 — Geometry and transformations (Ch. 3).**
  Vectors, points, normals, rays, bounding boxes,
  transformations, interactions. Blocks: everything.
  Tests: geometry tests in TEST_PLAN §1.
- **T2 — Monte Carlo basics (Ch. 2, scoped).** Random
  sampling, inversion method, distribution transforms,
  only to the depth Ch. 13 needs (per T0 verification).
  Blocks: T5 (BSDF sampling) and T6 (path tracer).
  Tests: Monte Carlo tests in TEST_PLAN §1.
- **T3 — Cameras and film (Ch. 5).** Camera interface,
  projective camera, film accumulation and image
  output in the chosen format. Spherical camera is a
  stretch goal, not part of this ticket. Blocks: T6.
  Tests: camera/film tests in TEST_PLAN §1.
- **T4 — Shapes and BVH (Ch. 6 + 7).** Shape interface,
  sphere, cylinder, disk, triangle mesh (must-have;
  bilinear patch and curve are stretch goals),
  rounding-error handling, primitive interface,
  aggregates, BVH. Blocks: T6. Tests: shape, BVH,
  and rounding-error tests in TEST_PLAN §1.
- **T5 — Reflection models (Ch. 9).** BSDF interface
  (sample / evaluate / PDF), diffuse, specular,
  conductor, dielectric, microfacet roughness, rough
  dielectric. Measured BSDFs and hair are excluded
  and are not tickets. Blocks: T6. Tests: reflection
  tests in TEST_PLAN §1.
- **T6 — Simple path tracer (Ch. 13).** Light transport
  equation, simple path tracer, CLI, default scene
  (fixed at code time, see SPEC §4), integration
  script. Blocks: T7. Tests: path-tracer and
  integration tests in TEST_PLAN §1–2.
- **T7 — Code review + push prep.** Review against
  SPEC, check no out-of-scope claims, run the full
  suite + a default-scene render for eyeballing, then
  present for per-push approval. Push itself is
  separate, on approval, and is not done by this
  docs subagent.

## Stretch goals (only after T6 passes, in this order)

- S1 — Spherical camera (Ch. 5).
- S2 — Bilinear patches and curves (Ch. 6).
- S3 — Better path tracer (Ch. 13).
- S4 — Minimum sampler from Ch. 8, **only** if T0
  verification showed Ch. 13's simple tracer needs it
  (see SPEC §3). Ch. 8 as a whole is still not in scope.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6 → T7, with T2 also
feeding T5/T6. T3 and T4 can be worked in either order
after T1, but both must pass before T6. There is no
parallel ticket here: each chapter's behaviour is the
foundation for the next, and a parallel build would
hide a regression in an earlier chapter behind a later
one. Out-of-scope chapters (8 in full, 10, 11, 12 in
full, 14, 15, 16, appendices) have no tickets at all.
