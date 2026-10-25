# TICKETS — Computer Graphics from Scratch (Gambetta)

Build order follows the book's chapters. The two sub-builds
are separate ticket series. Part I is done (code + tests
passing + reviewed) before Part II starts, except where a
ticket is explicitly shared. No ticket starts code for a
later chapter early, and no Part II ticket is used to fix
a Part I failure — a Part I failure is fixed in Part I.

## Shared tickets (both sub-builds)

- **T0 — Toolchain check.** Run the TEST_PLAN section 0
  check: JavaScript runtime version, browser/headless
  canvas availability, pixel-readback method, test
  framework or plain-script runner. Record the result.
  Blocks: everything. If this fails, stop and report —
  do not start T1 on an unverified toolchain.
- **T1 — Canvas, colour, and math (chapter 1 + appendix).**
  `canvas.js` (PutPixel + pixel readout), `color.js`,
  `math.js` (vectors, matrices, one fixed coordinate
  convention). Blocks: all Part I and Part II tickets.
  Tests: shared-core tests in TEST_PLAN section 2.

## Part I tickets — Raytracer (chapters 2–5)

- **T2 — Basic raytracing (chapter 2).** Rays, sphere
  intersection, nearest hit, background colour. Render
  loop that produces flat-coloured spheres. Blocks: T3–T5.
  Tests: intersection tests, basic-render pixel test.
- **T3 — Light (chapter 3).** Ambient, point, and
  directional lights; diffuse + specular shading with a
  per-material exponent. Blocks: T4. Tests: hand-computed
  light cases, exponent 500 vs. 10 highlight test.
  Verification note: chapter 3's body beyond its opening
  sections was not read in full — verify its light details
  against the chapter before closing this ticket, and
  record any correction in SPEC/DESIGN.
- **T4 — Shadows and reflections (chapter 4).** Shadow
  test per light; recursive reflection with the blend
  formula and a recursion-depth limit. Blocks: T5.
  Tests: shadow test, reflection blend test, depth-limit
  termination test. Verification note: the shadow-ray
  `t` range and any self-shadowing epsilon are to be
  verified against chapter 4 before closing this ticket.
- **T5 — Part I review.** Review Part I against SPEC
  sections 2–3, run the full Part I test suite, render
  the default sphere scene for eyeballing, and record
  any still-unverified chapter detail as an open item.
  This ticket closes Part I. It does not start Part II.

## Part II tickets — Rasterizer (chapters 6–14)

- **T6 — Lines and filled triangles (chapters 6–7).**
  Line drawing, then triangle filling with a single
  colour. Blocks: T7–T12. Tests: line tests, filled-
  triangle tests (interior, exterior, shared edge).
- **T7 — Shaded triangles (chapter 8).** Colour gradient
  across a triangle by interpolation. Blocks: T8. Tests:
  shaded-triangle gradient test.
- **T8 — Perspective projection and scene (chapters
  9–10).** Projection, Model → Instance → Camera
  transforms, default cube scene rendering. Blocks:
  T9–T12. Tests: perspective size-ratio test, default
  cube render test.
- **T9 — Clipping (chapter 11).** Instance-level and
  triangle-level clipping. Blocks: T10. Tests: cube
  behind camera, cube straddling the boundary, separate
  instance-level and triangle-level cases.
- **T10 — Hidden surface removal (chapter 12).** Depth
  buffer and back-face culling, both toggleable.
  Blocks: T11. Tests: draw-order independence at
  overlaps, culling on/off tests.
- **T11 — Shading and textures (chapters 13–14).** Flat,
  Gouraud, and Phong shading modes; textures with
  perspective-correct interpolation (and the linear mode
  as a test contrast, not as the shipped behaviour).
  Blocks: T12. Tests: three shading modes differ,
  highlight moves with the light, texture perspective-
  correctness test (the most important Part II test).
  Verification note: which normal each shading mode
  uses, and the exact texture interpolation detail, are
  to be verified against chapters 13–14 before closing
  this ticket.
- **T12 — Part II review.** Review Part II against SPEC
  section 4, run the full Part II test suite plus the
  comparison test (TEST_PLAN section 5), render the
  default cube scene for eyeballing, and record any
  still-unverified chapter detail as an open item.
  This ticket closes Part II.
- **T13 — Whole-book review (both parts, after T5 and
  T12).** Check the two renderers against the book's
  own comparison (same simple scene, raytracer strengths
  vs. rasterizer strengths, no pixel-equality claim),
  confirm no out-of-scope claim (refraction,
  anti-aliasing, rasterizer shadows, GPU) appears in code
  or docs, and present for per-push approval. Push
  itself is separate, on approval.

## Dependencies between tickets

```
T0 → T1 → T2 → T3 → T4 → T5 (Part I complete)
                    ↓
              T6 → T7 → T8 → T9 → T10 → T11 → T12 (Part II complete)
                    ↓
              T13 (whole-book review)
```

T1 is the only shared foundation. Part II does not depend
on Part I's shading code — the rasterizer's shading is
its own implementation (chapters 8 and 13), not a reuse of
the raytracer's chapter 3 code — and Part I does not depend
on Part II. That separation is deliberate: it is what
makes each part independently reviewable, and it is what
makes "both" honestly two projects' worth of work.
