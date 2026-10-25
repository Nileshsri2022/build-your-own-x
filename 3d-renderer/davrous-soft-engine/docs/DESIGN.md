# DESIGN — 3D Soft Engine (Davrous)

## Layout

One engine, three parallel implementations, same structure
in each language. Names follow the tutorial's objects so the
design stays traceable; implementations are written from
SPEC, not copied from the tutorial.

```
# C# (modern .NET, writable-bitmap presentation)
src/cs/
  Camera.cs       # position + target
  Mesh.cs         # name, vertices, position, rotation, faces, normals, uvs
  Device.cs       # back buffer, depth buffer, render loop, present
  Rasterizer.cs   # line drawing, scanline fill, ProcessScanLine, Interpolate, Clamp
  Texture.cs      # fixed-size image buffer, texel sampling with modulo wrap
  Program.cs      # demo scenes (cube, Suzanne, torus), frame loop wiring

# TypeScript (compiled to plain JS, HTML5 canvas)
src/ts/
  camera.ts, mesh.ts, device.ts, rasterizer.ts, texture.ts, main.ts
  # same objects and same function names as the C# side, in TS idiom

# JavaScript (plain, no build step required for the demo page)
src/js/
  camera.js, mesh.js, device.js, rasterizer.js, texture.js, main.js
  index.html      # canvas page that runs the demo scenes
```

A shared test harness (language-agnostic, see TEST_PLAN)
drives all three implementations through the same scenes and
compares captured back buffers. The three implementations
must agree pixel for pixel on the same scene and frame, or
the difference is a bug in one of them, not "language
variation".

## Objects

- **Camera.** Two 3D vectors: position and target. Derived
  at render time: view matrix (from position/target) and
  projection matrix (field of view 0.78, near 0.01, far 1.0).
  No other camera controls are in scope.
- **Mesh.** Name; a list of vertices (each a 3D position);
  a list of faces (each three vertex indices); per-vertex
  normals; per-vertex texture coordinates (from Blender
  JSON where present; absent for the hand-built cube);
  a world position and a rotation. World transform =
  translation × rotation (order to be pinned in code from
  the tutorial's matrix usage, and tested by the rotation
  test in TEST_PLAN — if the cube rotates the wrong way,
  the order is wrong).
- **Device.** Owns the back buffer (a pixel array at the
  render resolution), the depth buffer (a per-pixel depth
  array, same resolution), and the render loop. Its
  `render(camera, meshes)` does, per frame: clear both
  buffers; for each mesh, transform vertices
  (world × view × projection), project to screen space,
  and hand each face to the rasterizer; present the buffer.
- **Texture.** A fixed-size image buffer plus a sampler:
  given (u, v), wrap both coordinates with modulo, then
  return the texel. Wrapping, not clamping, is the taught
  behaviour, and the wrap test in TEST_PLAN pins it.
- **Face / triangle (internal).** Three projected screen
  vertices, each carrying screen x/y, depth, normal, and
  texture coordinates, so the scanline stage can interpolate
  all of them together.

## Rendering loop (per frame)

```
clear(backBuffer, backgroundColour)
clear(depthBuffer, farValue)          # farValue: to verify before code, see SPEC §5
for each mesh in scene:
  worldMatrix = translation(mesh.position) * rotation(mesh.rotation)
  for each face in mesh.faces:
    project its 3 vertices: screen = project(world * view * projection * vertex)
    if backFaceCulling enabled and face faces away: skip
    rasterize(face)
present(backBuffer)
```

`rasterize(face)`:

```
if mode == wireframe:
  draw 3 lines with Bresenham (Part 2 final method)
else:
  sort face vertices by screen Y
  walk edges with inverse slopes, scanline by scanline
  for each scanline: ProcessScanLine(left, right, interpolated depth/normal/uv)
    for each pixel x on the scanline:
      if depthBuffer[x, y] already holds a nearer depth: skip
      colour = shade(pixel)   # flat: face shade; Gouraud: interpolated normal
      if texture present: colour = colour * texel(uv)
      backBuffer[x, y] = colour
      depthBuffer[x, y] = depth
```

`shade(pixel)` (Part 5): light contribution =
`colour * max(0, dot(normal, lightDirection))`, where the
light is a single diffuse point light with no falloff.
Flat mode uses the face normal for every pixel of the face;
Gouraud mode uses the per-pixel interpolated normal.
**To verify before code:** whether the tutorial's light
direction is a unit vector from the surface to the light or
a fixed direction, and whether the shade is applied before
or after texture sampling — both affect pixel values, and
TEST_PLAN's flat-vs-Gouraud tests must pin them before the
shading ticket is considered done.

## Numbers and determinism

- All geometry math in double precision (C# `double`,
  TypeScript/JavaScript `number`, which is double).
- Pixel colours are 8 bits per channel in the back buffer.
  The conversion from a shaded colour in [0,1] to a byte is
  specified once, in code, and shared by all three
  implementations; a one-off difference here would fail the
  cross-language pixel test, which is the point of having it.
- Frames are deterministic for a fixed scene, camera, and
  rotation step: the same frame number must produce the
  same back buffer, in all three languages. Randomness is
  not used anywhere in the engine, so there is no seed to
  manage. Animation advances by a fixed rotation step per
  frame in the demo scenes, so frame N is reproducible.
- Render resolution is a parameter of the Device, not of
  the scene. Tests use a small resolution (for example
  200×200) so captured frames are cheap to compare; demos
  may use a larger one.

## Failure modes

- A Blender JSON file that is missing, malformed, or lacks
  vertices/faces: the loader reports the error and the
  program exits (or, in the browser demo, shows the error)
  without presenting a half-rendered frame as a success.
  A mesh with faces referencing out-of-range vertices is
  the same class of error.
- A texture file that cannot be loaded: same rule — error,
  no half-rendered frame.
- A frame that renders with no meshes is not an error: it
  presents a cleared back buffer. That is correct, and is
  tested as such.
- Parallelism (ticket T6, C# only) must not change any
  pixel: if a parallel frame differs from the serial frame
  for the same scene, the parallel code is wrong, however
  fast it is.
