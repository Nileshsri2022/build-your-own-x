# TICKETS — 3D Renderer

Build order follows the tutorial's lessons. Each ticket is
done (code + its tests passing) before the next starts, except
where noted. No ticket starts code for a later lesson early.

- **T1 — Vectors, rays, camera (lesson 0–1).** `Vec3`, `Ray`,
  `Camera` (bilinear interpolation, pixel-centre rays).
  Blocks: everything. Tests: Vec3Test, camera ray directions
  (centre ray points forward, corner rays point to corners).
- **T2 — Spheres and visibility (lesson 2).** `Sphere`
  intersection, nearest-hit selection, background colour,
  `Renderer` skeleton that renders unshaded spheres (hit =
  material colour, miss = background) so the image is
  checkable before shading exists. Blocks: T3–T6.
  Tests: SphereTest, RendererTest empty scene + unshaded
  centre pixel.
- **T3 — Phong shading (lesson 3).** `Material`, `Light`,
  `Color`, phong in `Renderer.trace` (no shadows yet).
  Blocks: T4. Tests: ShadingTest diffuse/specular cases.
- **T4 — Shadows (lesson 4).** Shadow ray per light, epsilon
  offset, blocked-light test. Blocks: T5.
  Tests: ShadingTest shadow case (ambient only when blocked).
- **T5 — Reflections (lesson 5).** Recursive `trace` with
  depth limit 3 and reflectivity blend. Blocks: T6.
  Tests: reflection blend / reflective scene case.
- **T6 — Anti-aliasing + CLI + PNG (lesson 6).**
  Jittered samples (seeded, deterministic), `S` CLI argument,
  `Main` argument validation, `PngWriter`, integration script.
  Blocks: T7. Tests: RendererTest AA + determinism,
  integration script (exit codes, PNG validity, centre/corner).
- **T7 — Code review + push prep.** Review against SPEC,
  check no out-of-scope claims, run the full suite + a
  `400 x 400` render for eyeballing, then present for
  per-push approval. Push itself is separate, on approval.

## Dependencies between tickets

T1 → T2 → T3 → T4 → T5 → T6 → T7, strictly linear. There is
no parallel ticket here: each lesson's behaviour is the
foundation for the next, and a parallel build would hide a
regression in an earlier lesson behind a later one.
