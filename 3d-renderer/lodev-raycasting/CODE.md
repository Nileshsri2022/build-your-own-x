# Code — Lodev Raycasting

Honest scope: this is a **raycaster** (semi-3D first-person
walker), NOT a true 3D renderer. No spheres, triangles,
meshes, Phong, shadows, reflections, refraction, floors,
ceilings, sprites, stairs, or height differences.

## Files (code only; the docs in `docs/` were not edited)

- `src/raycaster.h` — world maps, player/camera, DDA
  `castStripe`, untextured + textured `renderFrame` into a
  640x480 packed-RGB buffer, textures, movement/collision,
  rotation, FPS. Single header, no dependencies.
- `src/main.cpp` — headless driver: renders one frame and
  writes a binary PPM (P6). Options: `--mode
  untextured|textured`, `--output <file>`, `--pos X Y`,
  `--dir DX DY` (plane derived perpendicular, length 0.66).
- `src/tests.cpp` — behaviour test harness (no framework),
  mirroring `docs/TEST_PLAN.md`. Run with `make test`.
- `Makefile` — `make` (builds `raycaster` + `tests`),
  `make test`, `make run` (writes PPMs to `/tmp`).

## Display backend decision (docs left this open)

**Headless framebuffer → PPM file only.** No window mode
is built, and none is claimed as tested: this VM has no
GUI toolkit/window to run one in, and a window that was
never run would be a false claim. The framebuffer is the
same buffer a window backend would present, so every
pixel behaviour in the TEST_PLAN is checkable from it.
A future window backend would only replace `main.cpp`'s
PPM write with a present step; `raycaster.h` is unchanged.

## Recorded choices (from SPEC/DESIGN, confirmed in code)

- Zero ray components: IEEE-754 infinity (C++), not 1e30.
- `perpWallDist`: DESIGN simplified form (last side
  distance minus one delta), consistent with the delta form.
- Collision: per axis, x then y (point check only).
  Sliding along a wall at a corner is possible.
- Texture mapping: affine, correct for vertical walls,
  not perspective-correct in general.
- DDA cap: 10,000 steps; shipped maps are border-walled,
  so it is never reached. Border guarantee is tested
  (all 640 stripes terminate within 100 steps).
- Y-side darkening: `(colour >> 1) & 8355711`
  (0x7F7F7F mask prevents channel bleed).
- Maps: `untexturedMap()` (wall number 1) and
  `texturedMap()` (numbers 1–8), both 24x24, border-walled,
  defined in code in `src/raycaster.h` (no map files).
- Textures: 8 procedural 64x64 formulas, generated in
  code, never loaded from files.

## Toolchain (verified here, T0)

g++ 13.3.0 (`/usr/bin/g++`), C++17, no libraries beyond
the standard library.

## Unverified / not done (honest)

- No interactive window run: movement/rotation/collision
  are tested through the headless harness and the
  framebuffer, not by a human pressing keys in a window.
  The TEST_PLAN's "recorded interactive run" item is
  therefore NOT claimed.
- No floors, ceilings, sprites, or Doom-style advances —
  out of scope by design, not missing work.
- The tutorial's exact maps are not reproduced
  cell-for-cell (docs said not to); the shipped maps
  follow its structure (border + rooms/pillars/corridors).
