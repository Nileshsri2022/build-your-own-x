# TICKETS — Lodev Raycasting

Build order follows the tutorial's sections. Each ticket
is done (its tests passing) before the next starts, except
where noted. No ticket starts work for a later section
early, because each section's behaviour is the foundation
for the next.

- **T0 — Toolchain and display backend check (blocks
  everything).** Confirm a working C++ compiler and a
  display backend on the target machine; record both in
  DESIGN. If either is missing, stop and report the
  blocker — do not write rendering code against an
  unverified backend. Tests: the toolchain check in
  TEST_PLAN.
- **T1 — World and camera (tutorial: introduction,
  camera model).** Grid map (24x24, border-walled),
  player `pos`/`dir`/`plane`, cameraX mapping, rotation
  of `dir` and `plane` together. Blocks: everything
  else. Tests: cameraX mapping, perpendicularity,
  FOV, rotation round-trip.
- **T2 — DDA and untextured stripes (tutorial:
  DDA, untextured raycaster).** Per-stripe DDA loop,
  `perpWallDist`, line height and draw range,
  vertical-line draws, y-side darkening, frame-time
  movement and rotation, point collision, FPS counter.
  Blocks: T3–T4. Tests: DDA first-wall, no-skipped-cell,
  zero-component, border guarantee, wall-height vs
  distance, no-fisheye, centring/capping, side
  darkening, map fidelity, frame-time independence,
  collision, FPS.
- **T3 — Textured raycaster (tutorial: textured
  raycaster).** 2D screen buffer, 64x64 procedural
  textures, second map with texture numbers 1–8,
  `wallX` -> `texX`, vertical texture stepping,
  packed-colour y-side darkening with the mask.
  Blocks: T4. Tests: texture selection, wallX mapping,
  vertical stepping, affine-limit note, packed-colour
  darkening. The untextured build from T2 must still
  pass its own tests after T3 — T3 does not replace
  T2's behaviour silently.
- **T4 — Code review + push prep.** Review against
  SPEC, check no out-of-scope claims (no floors,
  ceilings, sprites, stairs, height differences, or
  Doom-style advances), run the full suite plus a
  recorded interactive run (movement, rotation,
  collision, quit), then present for per-push
  approval. Push itself is separate, on approval.

## Dependencies between tickets

T0 → T1 → T2 → T3 → T4, strictly linear. T0 is not
skippable: it is the only ticket that can honestly
answer "can this be built and run here?" before code
exists. There is no parallel ticket here: the
textured raycaster is the untextured raycaster plus
textures, and testing them in parallel would hide a
regression in the DDA core behind a texture bug.
