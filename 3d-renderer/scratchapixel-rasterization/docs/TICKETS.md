# TICKETS — Scratchapixel Rasterization

Build order follows the tutorial’s chapters. Each ticket is
done (code + its tests passing) before the next starts,
except where noted. No ticket starts code for a later
chapter early.

- **T0 — Pre-code verification (gate, not code).**
  Toolchain check (a C++ compiler that compiles a trivial
  program, version recorded) and re-fetch of the unverified
  items in SPEC §6 (Depth Buffer chapter; tails of the
  projection, rasterization, and concluding chapters).
  Blocks: everything. Nothing is "tested" until T0 passes.
  If T0 fails (no compiler, or a chapter that contradicts
  SPEC), SPEC is corrected first and the correction is
  recorded — code is not written against an unverified
  assumption.
- **T1 — Frame buffer and PPM writer (chapter 1 setup).**
  Frame buffer, background fill, PPM writer (the DESIGN
  variant, P3 or P6), CLI argument handling.
  Blocks: T2–T6. Tests: TEST_PLAN §1 (format, empty scene,
  bad arguments).
- **T2 — Projection (chapter 2).** Camera → screen
  (perspective divide, keep z), screen → NDC [-1, 1],
  NDC → raster coordinates, near-plane handling.
  Blocks: T3–T6. Tests: TEST_PLAN §5 (perspective size,
  off-centre position, behind-near-plane).
- **T3 — Triangle rasterization (chapter 3).**
  Edge function (Pineda), inside/outside test, top-left
  rule, bounding-box iteration, barycentric interpolation
  of vertex colour.
  Blocks: T4–T6. Tests: TEST_PLAN §§2–3 (coverage,
  bounding box, clipping, shared edge, vertex colours,
  centroid).
- **T4 — Depth buffer (chapter 4).** Depth array
  (initialised to infinity), depth test, nearest wins.
  Only the behaviour verified in T0 may be implemented;
  if the chapter is still unverified at this point, T4
  implements only the overview-level behaviour (float
  array, nearest wins, constant-depth cases) and the
  perspective-depth tests stay marked unverified.
  Blocks: T5–T6. Tests: TEST_PLAN §4 (nearer wins,
  order independence, equal depth, determinism).
- **T5 — Practical implementation (chapter 5).**
  Perspective-correct interpolation of attributes
  (st coordinates carried as data), and — only as a
  later, optional sub-ticket, after the pixel tests pass
  — 8×8 pixel blocks, incremental edge-function
  updates, and fixed-point coordinates. Anti-aliasing
  by sub-pixel sampling is an exercise in the lesson,
  not a shipped feature: it is NOT built in T5 unless
  the user explicitly asks for it as a separate piece
  of work. Blocks: T6. Tests: TEST_PLAN §6
  (determinism, reference PPM); optimisation sub-ticket
  must not change any pixel test result.
- **T6 — Code review + push prep.** Review against SPEC
  (triangles only, no shading claimed, no texture
  sampling claimed, no anti-aliasing claimed, Depth
  Buffer chapter status stated), run the full suite +
  the default scene render for review, then present for
  per-push approval. Push itself is separate, on approval.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6, strictly linear. Each
chapter’s behaviour is the foundation for the next, and a
parallel build would hide a regression in an earlier stage
behind a later one.
