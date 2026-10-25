# DESIGN — Lodev Raycasting

## Layout (proposed, to be confirmed at code time)

The tutorial presents its code as one long listing, not as
separate modules. These docs keep that shape for the core
loop but name its pieces, so tests can target them
without pretending the tutorial was modular:

```
main program
  WorldMap        # 24x24 int grid, 0 = empty, >0 = wall (and texture number)
  Player          # pos (x, y), dir (x, y), plane (x, y)
  RayState        # per-stripe DDA state: mapX/Y, side/dist/step, hit, side
  Textures        # 64x64 procedural textures, packed RGB integers
  ScreenBuffer    # 640x480 int buffer (textured version) / line draws (untextured)
  castStripe(x)   # cameraX -> rayDir -> DDA loop -> perpWallDist -> lineHeight/draw range
  renderFrame()   # for every stripe: castStripe, draw wall (line or textured column)
  handleInput()   # movement/rotation with frameTime, point collision, quit
  mainLoop()      # frameTime, FPS counter, renderFrame, present buffer
```

No file names or class names are fixed here, because the
tutorial does not fix them; the code phase must pick names
and record them. What is fixed is the *separation*:
world data, player/camera state, per-stripe DDA state,
texture data, and the frame loop are distinct concerns,
even if they live in one file.

## Per-stripe algorithm (pseudocode, not tutorial code)

```
cameraX = 2 * x / screenWidth - 1
rayDirX = dirX + planeX * cameraX
rayDirY = dirY + planeY * cameraX

mapX = int(posX); mapY = int(posY)
deltaDistX = (rayDirX == 0) ? INF : sqrt(1 + (rayDirY*rayDirY) / (rayDirX*rayDirX))
deltaDistY = (rayDirY == 0) ? INF : sqrt(1 + (rayDirX*rayDirX) / (rayDirY*rayDirY))
// INF is either IEEE-754 infinity (C++/Java/JS) or 1e30 (generic guard, e.g. Python).
// The code phase must state which one it uses; see SPEC section 4.

stepX = (rayDirX < 0) ? -1 : 1
sideDistX = (rayDirX < 0) ? (posX - mapX) * deltaDistX
                         : (mapX + 1.0 - posX) * deltaDistX
stepY = (rayDirY < 0) ? -1 : 1
sideDistY = (rayDirY < 0) ? (posY - mapY) * deltaDistY
                         : (mapY + 1.0 - posY) * deltaDistY

hit = false
while (!hit):
    if (sideDistX < sideDistY):
        sideDistX += deltaDistX; mapX += stepX; side = 0
    else:
        sideDistY += deltaDistY; mapY += stepY; side = 1
    if (worldMap[mapX][mapY] > 0): hit = true

// Perpendicular distance: use the side distance that was
// advanced last, minus the delta just added, so the result
// is the distance to the wall, not past it.
perpWallDist = (side == 0) ? (sideDistX - deltaDistX)
                           : (sideDistY - deltaDistY)
lineHeight = screenHeight / perpWallDist
drawStart = max(0, -lineHeight / 2 + screenHeight / 2)
drawEnd   = min(screenHeight - 1, lineHeight / 2 + screenHeight / 2)
```

Notes on the pseudocode above:

- The `sideDist` initial values use the fractional part of
  the player position, not the whole position. Getting
  this wrong is the classic DDA bug: walls appear shifted
  or the ray starts inside the wrong cell.
- `perpWallDist` as written here is the simplified form
  the tutorial derives (side distance minus one delta).
  It equals the perpendicular distance only because the
  delta values are used consistently; if the code uses
  the unsimplified Euclidean delta form, it must derive
  `perpWallDist` from the same form and say so.
- The loop has no explicit iteration cap in the tutorial,
  because the map is border-walled: every ray must hit a
  wall. The code phase must keep border walls, or add a
  cap and an "escaped the map" failure — it must not
  silently allow an infinite loop on an open map.

## Textured column (per stripe, after the DDA loop)

```
texNum = worldMap[mapX][mapY] - 1        // texture index from wall number
wallX = (side == 0) ? posY + perpWallDist * rayDirY
                    : posX + perpWallDist * rayDirX
wallX -= floor(wallX)                     // fractional part only
texX = int(wallX * texWidth)
step = texHeight / lineHeight
texPos = (drawStart - screenHeight / 2 + lineHeight / 2) * step
for y in drawStart..drawEnd:
    texY = int(texPos) & (texHeight - 1)  // only valid because texHeight is a power of two (64)
    texPos += step
    colour = textures[texNum][texY * texWidth + texX]   // layout depends on storage order; record it
    if (side == 1): colour = (colour >> 1) & 8355711     // y-side darkening, mask explained in SPEC
    buffer[y][x] = colour
```

The `& (texHeight - 1)` wrap and the darkening mask are
both places where a magic number must be commented in
code: the first only works for power-of-two texture
sizes, the second prevents packed RGB channels from
bleeding into each other when shifted.

## Frame loop and timing

```
previousTime = now()
loop:
    frameTime = (now() - previousTime) / 1000.0   // seconds
    previousTime = now()
    fps = 1.0 / frameTime                          // guard against frameTime == 0
    handleInput(moveSpeed * frameTime, rotSpeed * frameTime)
    renderFrame()                                  // all stripes, into the buffer
    present(buffer)                                // backend-specific; see below
```

Movement (point collision, per axis — record whether the
code checks x and y together or separately, because the
two behave differently at corners):

```
newX = posX + dirX * moveSpeed * frameTime   // forward; subtract for backward
if (worldMap[int(newX)][int(posY)] == 0): posX = newX
newY = posY + dirY * moveSpeed * frameTime
if (worldMap[int(posX)][int(newY)] == 0): posY = newY
```

Rotation (same angle for `dir` and `plane`):

```
oldDirX = dirX
dirX = dirX * cos(rot) - dirY * sin(rot)
dirY = oldDirX * sin(rot) + dirY * cos(rot)
oldPlaneX = planeX
planeX = planeX * cos(rot) - planeY * sin(rot)
planeY = oldPlaneX * sin(rot) + planeY * cos(rot)
```

## Display backend — decision required before code

The render logic above ends at a buffer (or, in the
untextured version, at line draws). Before code, the code
phase must record:

1. Which window/screen backend is used on the target
   machine, and how `present(buffer)` is implemented.
2. Which keys move/rotate/quit, and how input is polled
   per frame.
3. Whether the untextured version is kept as a separate
   build/mode or replaced by the textured version
   (the tutorial presents both; these docs keep both as
   tickets T3 and T4, in that order).

No backend is chosen in these docs, so no backend claim
can be quoted from them later.

## Failure modes

- An open (unbordered) map: rays can leave the grid.
  Prevented by border walls (tutorial's approach) or by
  an iteration cap with a visible failure — never by an
  unbounded loop.
- A zero ray component: handled by the INF / 1e30 guard
  in SPEC section 4, not by special-casing every stripe.
- A wall number with no texture (textured version):
  the code must fail visibly at load/map-check time, not
  draw a wrong texture silently.
- `frameTime == 0`: the FPS computation must guard
  against division by zero.
- A non-perpendicular camera plane: not checked at
  runtime in the tutorial; the code phase should assert
  or document it at setup time if it adds such a check,
  and must not silently "fix" a skewed plane.
