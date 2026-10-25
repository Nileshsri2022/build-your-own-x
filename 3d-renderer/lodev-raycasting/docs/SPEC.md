# SPEC — Lodev Raycasting (Wolfenstein-style)

## 1. Idea

The world is a flat 2D grid. The player stands in one cell
and looks in a direction. The screen is divided into vertical
stripes — one per screen column (640 at 640x480). For each
stripe, cast one ray from the player through the camera plane
and walk it through the grid until it hits a wall cell. The
distance to that wall decides how tall the wall stripe is
drawn on screen. That is the whole rendering method: there
is no 3D geometry, no triangles, no spheres, and no lighting
model beyond the darkening described in section 5.

This is why the result is semi-3D, not true 3D: every wall
has the same height, walls are orthogonal squares on the
grid, and the camera can never look up or down, climb
stairs, or jump. Those limits are the tutorial's, and this
build keeps them.

## 2. The world

- A grid of 24 x 24 cells, stored as a 2D array of integers.
- `0` means empty, walkable space. A positive integer means
  a wall; in the textured version the number also selects
  the wall's texture (numbers 1–8 in the tutorial's second
  map).
- The tutorial's first (untextured) map is a large room with
  a wall border, a small inner room, pillars, a corridor and
  a room. The second (textured) map is a different, more
  complex arrangement of corridors and rooms. These docs do
  not reproduce either map cell-for-cell here; the code
  phase must define one map for the untextured build and one
  for the textured build, following the tutorial's structure
  (border walls so the player can never walk off the grid,
  plus interior rooms/pillars/corridors), and record both
  maps in the code's own documentation.
- Coordinates: a cell is 1 unit wide. The player's position
  is a floating-point (x, y) point inside a cell. The ray
  position itself is floating point and carries both which
  cell the ray is in and where inside that cell it is.

## 3. The camera

The player is described by three vectors, all in the 2D plane:

- `pos`: the player's position (x, y).
- `dir`: the direction the player looks. Its length does not
  matter, only its direction.
- `plane`: the camera-plane vector. It must be perpendicular
  to `dir`. It represents the screen surface: the point
  `pos + dir` is the centre of the screen, `pos + dir - plane`
  is the left edge, `pos + dir + plane` is the right edge.

For a screen column `x` (0-based, out of `screenWidth`
columns), the camera-plane coordinate is:

```
cameraX = 2 * x / screenWidth - 1     // -1 at the left edge, 0 in the centre, +1 at the right edge
rayDir = dir + plane * cameraX
```

The angle between the two outer rays is the field of view
(FOV). It is set by the ratio of the lengths of `dir` and
`plane`: equal lengths give 90 degrees; a much longer `dir`
gives a narrow, zoomed-in view; a much shorter `dir` gives a
wide view (up to 180 degrees as `dir` approaches 0).
Rotation rotates both `dir` and `plane` by the same angle,
using the 2D rotation matrix, so the rays rotate with the
player. A camera plane that is not perpendicular to `dir`
is not forbidden by the maths, but it makes the world look
skewed, and this build must keep them perpendicular.

## 4. DDA: finding the first wall

Stepping a ray forward by a constant amount can miss a wall
entirely (the ray passes through a wall cell between two
checked points), and making the step smaller only reduces
the chance of missing, never removes it. The tutorial's
solution is DDA (Digital Differential Analysis): instead of
stepping along the ray, step from one grid line to the next,
so the ray can never jump over a cell.

Per stripe, the DDA state is:

- `mapX`, `mapY`: the integer cell the ray is currently in
  (initially the player's cell).
- `deltaDistX`, `deltaDistY`: the distance the ray must
  travel to go from one x-side to the next x-side, and from
  one y-side to the next y-side. Geometrically, with a cell
  width of 1:
  `deltaDistX = sqrt(1 + (rayDirY^2) / (rayDirX^2))`
  `deltaDistY = sqrt(1 + (rayDirX^2) / (rayDirY^2))`
  Only the *ratio* between the two matters for the stepping
  decision, so the code may use the simplified form in
  which the ray length is taken as 1 — but it must record
  that simplification, because the delta values then do not
  match the lengths drawn in the tutorial's diagrams.
- `sideDistX`, `sideDistY`: the distance from the ray start
  to the first x-side and first y-side in the ray's
  direction. Which side is "first" depends on the sign of
  the ray direction, and the integer cell coordinate minus
  the fractional position (plus 1.0 in the positive case)
  gives it.
- `stepX`, `stepY`: +1 or -1, the direction of travel in
  each axis, from the sign of the ray direction. A zero
  component makes its step unused, not an error.
- `hit`: false until a wall cell is entered. `side`: 0 if
  an x-side was hit, 1 if a y-side was hit.

The loop: at each step, advance along whichever of
`sideDistX` / `sideDistY` is smaller, incrementing that
side distance by the matching delta and moving `mapX` or
`mapY` by the matching step, until the cell at
(`mapX`, `mapY`) is a wall. Then `hit` is true.

Division-by-zero guard: if `rayDirX` or `rayDirY` is 0, the
corresponding delta would divide by zero. The tutorial's
generic guard sets it to a very large value (`1e30`).
C++, Java and JavaScript produce IEEE-754 infinity for
such a division, which also works correctly in the DDA
comparison; a Python port must use the explicit `1e30`
guard. The code phase must state which form it uses and
why, for the language it is written in.

## 5. From distance to a stripe on screen

Do **not** use the straight-line (Euclidean) distance from
the player to the hit point. That distance is larger for
rays at the screen edges than for the centre ray, so a
straight-on wall would be drawn with rounded, varying
heights — the fisheye effect. Instead use the
*perpendicular* wall distance: the distance from the hit
point to the camera plane, i.e. the ray length projected
onto the camera direction. With that distance, all stripes
of a straight-on wall have the same length, and the wall's
top and bottom are horizontal lines.

Call it `perpWallDist`. Then:

```
lineHeight = screenHeight / perpWallDist
drawStart = max(0, screenHeight / 2 - lineHeight / 2)
drawEnd   = min(screenHeight - 1, screenHeight / 2 + lineHeight / 2)
```

The wall stripe is drawn from `drawStart` to `drawEnd`,
centred vertically. Stripes outside the screen are capped
to `[0, screenHeight - 1]`. A multiplier on the height
(e.g. `2 * screenHeight`) is allowed as a tuning choice
for taller-looking walls, but it must be recorded as a
choice, not presented as the tutorial's default.

Untextured version: the stripe is drawn as a vertical line
in the wall's colour. The tutorial darkens y-side hits
relative to x-side hits, which reads as a crude light
effect. That side-darkening is the only shading in this
build.

Textured version: the vertical-line draw is replaced by
per-pixel drawing into a 2D screen buffer, copied to the
screen at once (faster than drawing pixel by pixel to the
screen). Extra data per stripe:

- Textures are 64 x 64 texels, stored as single integers
  packing R, G and B (not three separate bytes). In the
  tutorial the textures are procedurally generated, not
  loaded from files; the code phase must do the same
  unless it records a deliberate change.
- `wallX`: the exact hit position along the wall, i.e.
  the fractional world coordinate where the ray hit, with
  the integer part removed. For a y-side hit this is a
  y-coordinate, but it is always used as the texture's
  x-coordinate.
- `texX = int(wallX * texWidth)`, the texture column.
- `texY` is stepped down the stripe by a precomputed step
  (texture height divided by line height), one texel per
  screen pixel, and the pixel colour is
  `texture[texNum][texX][texY]`.
- This stepping is *affine* texture mapping: it
  interpolates linearly rather than computing a division
  per pixel. It is not perspective-correct in general, but
  for perfectly vertical walls (and perfectly horizontal
  floors/ceilings) it is correct, which is why it works
  here. That limitation must stay stated, not hidden.
- Y-side darkening again applies, but now on a packed
  integer colour: the tutorial halves R, G and B by
  right-shifting, with a mask to keep the channels from
  bleeding into each other (mask value 8355711 in the
  tutorial's code). The code phase must explain the mask
  in a comment, not leave it as a magic number.

## 6. Movement, collision, and timing

- Movement speed and rotation speed are multiplied by
  `frameTime` (the time since the previous frame), so
  speed is independent of processor speed.
- Forward/backward movement adds/subtracts `dir` (assumed
  normalised, length 1) times move speed times frame time.
  Left/right rotation rotates `dir` and `plane` by
  rotation speed times frame time.
- Collision detection is a single point check: a move is
  only applied if the destination cell is not a wall. The
  tutorial notes this can be improved (e.g. a circle
  around the player instead of a point); this build keeps
  the point check and records any improvement as a
  deliberate change, not as tutorial behaviour.
- An FPS counter is included, computed from frame time.

## 7. Display backend — open question, not a detail

The tutorial's rendering logic ends at a screen buffer
(textured version) or vertical-line draws (untextured
version). How that buffer becomes a window on a modern
machine is a code-phase decision: it must be chosen,
recorded in DESIGN before code, and its input handling
(arrow keys / WASD or equivalent, quit key) must match
the movement model in section 6. No backend is claimed
in these docs.

## 8. Non-goals (honest)

No floors or ceilings (textured or otherwise); no sprites;
no stairs, jumping, or height differences; no sloped or
transparent walls; no doors; no lighting model beyond
side-darkening; no perspective-correct texture mapping
beyond the vertical-wall case; no map loading from a file
in the tutorial's code (maps are defined in code — a file
format would be a separate feature, not a hidden one).
The Doom / Duke Nukem 3D-style advances the tutorial
mentions (sloped walls, different heights, textured floors
and ceilings, transparent walls) are explicitly not built.
