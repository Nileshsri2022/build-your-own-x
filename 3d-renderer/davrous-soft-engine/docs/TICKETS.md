# TICKETS — 3D Soft Engine (Davrous)

Build order follows the tutorial's parts. Each ticket is
done (code + its tests passing, in every language that
passed the toolchain check) before the next starts, except
where noted. No ticket starts code for a later part early.

- **T0 — Toolchain check (TEST_PLAN §0).** Confirm a
  current .NET / TypeScript / browser host for each
  language, with a minimal presenting program. Record the
  result; drop any language that fails, explicitly, in a
  docs revision. Blocks: everything. This ticket writes no
  engine code.
- **T1 — Core logic (Part 1).** Camera, Mesh, Device, back
  buffer, render loop, projection of the 8 cube points.
  Blocks: everything. Tests: projection/rotation, empty
  scene.
- **T2 — Wireframe (Part 2).** Line drawing (Bresenham as
  the shipped method; the recursive midpoint method may be
  built as a stepping stone and then replaced), triangle
  edges. Blocks: T3. Tests: line unit tests, wireframe cube.
- **T3 — Blender JSON import (Part 3).** Mesh loader for
  Blender JSON (vertices, faces, normals, UVs), Suzanne
  and torus demo scenes. Blocks: T4. Tests: import tests,
  non-empty render of both meshes.
- **T4 — Filled triangles + Z-Buffer (Part 4).** Scanline
  rasterization (`ProcessScanLine`, `Interpolate`, `Clamp`),
  depth buffer with the comparison pinned by test.
  **Pre-code verification inside this ticket:** the depth
  comparison direction and initial depth value (SPEC §5),
  confirmed by the depth-ordering tests before the ticket
  is marked done. Blocks: T5. Tests: filled triangle,
  depth ordering, texture-wrap unit test (the sampler is
  needed by T6, but its wrapping rule is pinned here).
- **T5 — Shading (Part 5).** Flat shading, then Gouraud
  shading, single diffuse point light, no falloff.
  **Pre-code verification inside this ticket:** the light
  direction convention and the shade/texture order
  (DESIGN), pinned by the flat-vs-Gouraud tests.
  Blocks: T6. Tests: flat vs Gouraud, light-reversed.
- **T6 — Textures + back-face culling + parallelism
  (Parts 6 and 4b).** Texture class and sampling with
  modulo wrap, combined shade × texel; back-face culling
  with the no-front-face-lost test; C# parallelism as a
  separate, pixel-identical sub-ticket after the serial
  path passes. **Pre-code verification inside this
  ticket:** texture-interpolation detail, culling code
  detail, and the WebGL comparison section (SPEC §6) —
  read the tutorial's Part 6 in full before writing this
  ticket's code. Blocks: T7. Tests: texture, wrap,
  combined shade × texel, culling, parallel pixel
  identity (C# only).
- **T7 — Demo, review + push prep.** Wire the demo scenes
  (cube, Suzanne, torus) in all languages, capture frames
  for eyeballing, review against SPEC (no out-of-scope
  claims, no copied tutorial code), run the full suite +
  cross-language agreement, then present for per-push
  approval. Push itself is separate, on approval.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4 → T5 → T6 → T7, strictly linear.
There is no parallel ticket here: each part's behaviour is
the foundation for the next, and the three languages move
together through each ticket, so a cross-language
difference is caught at the ticket that introduced it, not
at the end. Parallelism (Part 4b) is deliberately late,
inside T6, not parallel with the serial rasterizer: it must
not be built before the serial path it has to match.
