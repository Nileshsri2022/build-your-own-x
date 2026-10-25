# TICKETS — tinyrenderer

Build order follows the verified features, not a
claimed course-notes lesson order (that order is
unverified — see README). Each ticket is done (code
+ its tests passing) before the next starts, except
where noted.

- **T0 — Toolchain check (pre-code, blocks all).**
  Confirm a C++ compiler and CMake, record versions.
  If unavailable here, move the build/test to a
  machine that has them. No code ticket starts until
  T0 passes.
- **T1 — Framebuffer and pixels.** TGA-image class
  integration, framebuffer creation, setting single
  pixels, writing `framebuffer.tga`. Blocks: everything.
  Tests: framebuffer / all-background test, file
  validity and dimensions.
- **T2 — Triangle rasterization.** Barycentric
  inside/outside test, bounding-box iteration, solid
  colour fill. Blocks: T3–T8. Tests: single-triangle
  centre / outside / edge test.
- **T3 — Back-face culling and depth buffer.**
  Signed-area culling test, per-pixel depth buffer,
  nearest-wins rule. Blocks: T4–T8. Tests: reversed
  winding renders background; overlapping triangles
  in both submission orders.
- **T4 — Camera and projection.** Model-to-screen
  transform, mesh loading (`.obj`: vertices, UVs,
  faces). Blocks: T5–T8. Tests: camera-move position /
  size test; full render of a real mesh (unshaded or
  flat-coloured at this stage, so the image is
  checkable before shading exists).
- **T5 — Shading (Phong).** Local Phong shading
  (ambient + diffuse + specular), with coefficients
  fixed and recorded in this ticket. Blocks: T6–T8.
  Tests: shading relative-brightness / highlight test.
- **T6 — Textures.** Texture loading and sampling by
  interpolated UV coordinates. Blocks: T7–T8.
  Tests: quadrant-texture colour-order test.
- **T7 — Tangent-space normal mapping.** Tangent /
  bitangent / normal basis from the UV grid, per-pixel
  normal perturbation, shading with the perturbed
  normal. Blocks: T8. Tests: with/without normal map
  comparison test.
- **T8 — Shadow mapping.** Light-view depth pass,
  camera pass with hard binary shadow mask. Blocks: T9.
  Tests: occluder shadow test (dark behind occluder,
  lit outside, hard edge).
- **T9 — Code review + push prep.** Review against
  SPEC, resolve or explicitly carry forward every
  "to verify" item, run the full suite + a real-mesh
  render for eyeballing, then present for per-push
  approval. Push itself is separate, on approval.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6 → T7 → T8 → T9,
strictly linear. Each stage's behaviour is the
foundation for the next, and a parallel build would
hide a regression in an earlier stage behind a
later one.

## Explicitly not tickets here

Indirect lighting and toon shading: only as
unverified later lessons (see README). If a fresh
read of the course notes verifies them, they become
T10/T11 by a scope revision, not by silent addition.
