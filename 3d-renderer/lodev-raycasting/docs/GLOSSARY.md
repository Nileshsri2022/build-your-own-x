# GLOSSARY — Lodev Raycasting

- **Raycasting.** Casting one ray per screen column
  through a 2D grid world and drawing a vertical wall
  stripe per ray. Semi-3D: it looks 3D, but the world
  is flat and walls are all the same height. Not
  raytracing.
- **Raytracing.** Casting rays into a true 3D scene
  and computing reflections, shadows, and shading per
  hit. The tutorial contrasts this with raycasting;
  this project does not do it.
- **Grid / map.** The 2D array of cells that is the
  world. `0` = empty, a positive integer = a wall
  (and, in the textured version, a texture number).
- **Cell.** One square of the grid, 1 unit wide.
- **Player / pos.** The player's floating-point
  position inside a cell.
- **Direction vector (dir).** The unit-length vector
  the player looks along. Only its direction matters.
- **Camera plane (plane).** The vector perpendicular
  to `dir` that represents the screen. `pos + dir` is
  the screen centre; `pos + dir +/- plane` are its
  edges.
- **cameraX.** The coordinate of a screen column on
  the camera plane: -1 at the left edge, 0 in the
  centre, +1 at the right edge.
- **FOV (field of view).** The angle between the two
  outer rays, set by the ratio of the lengths of
  `dir` and `plane`.
- **DDA (Digital Differential Analysis).** Stepping
  from one grid line to the next, instead of stepping
  a fixed distance along the ray, so a wall cell can
  never be jumped over.
- **deltaDist.** The distance a ray travels to go
  from one grid line to the next in one axis
  (`deltaDistX` / `deltaDistY`).
- **sideDist.** The distance from the ray start to
  the first grid line in one axis
  (`sideDistX` / `sideDistY`).
- **step.** +1 or -1: the direction of travel along
  one axis (`stepX` / `stepY`).
- **side.** Which kind of grid line was hit last:
  0 for an x-side, 1 for a y-side. Y-side hits are
  darkened.
- **perpWallDist.** The perpendicular wall distance:
  the distance from the hit point to the camera
  plane, not the straight-line distance to the
  player. Using it avoids the fisheye effect.
- **Fisheye effect.** The rounding distortion that
  appears if straight-line distance is used: a
  straight-on wall draws with varying heights across
  the screen instead of a flat top and bottom.
- **lineHeight.** The height, in pixels, of a wall
  stripe: `screenHeight / perpWallDist`, capped to
  the screen.
- **wallX.** The exact fractional position along a
  wall where a ray hit, used as the texture's
  x-coordinate.
- **texX / texY.** The column / row inside a texture
  (64x64 texels) for a given screen pixel.
- **Affine texture mapping.** Interpolating texture
  coordinates linearly, without a per-pixel division.
  Correct for vertical walls; not perspective-correct
  in general.
- **Screen buffer.** A 2D array of pixel colours
  built up for a whole frame and then copied to the
  screen at once (used in the textured version).
- **Frame time.** The time since the previous frame.
  Movement and rotation speeds are multiplied by it
  so they do not depend on processor speed.
- **FPS counter.** Frames per second, computed as
  1 / frameTime.
- **Point collision.** Blocking a move if the
  destination point (a single point, not a circle
  around the player) would be inside a wall cell.
- **Sprite.** A 2D image enemy, object, or goodie in
  the world. Not built in this tutorial or project.
