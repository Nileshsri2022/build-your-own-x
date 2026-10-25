# SPEC — 3D Soft Engine (Davrous)

## 1. Idea

Build a 3D engine that uses only the CPU, in the old-school
way: no GPU rendering API does the drawing for us. Each frame,
the engine projects the vertices of a triangle mesh into 2D,
fills the triangles pixel by pixel into a back buffer, shades
them, and presents the buffer. Understanding this pipeline is
the point; for a real game, a GPU API would be the right tool,
and this engine does not pretend otherwise.

This is rasterization, not ray tracing. There is no ray per
pixel and no ray–object intersection. There is also no model
editing: the engine renders meshes that are built in code or
imported from Blender JSON files.

## 2. Parts (tutorial order)

- **Part 1 — Core logic: camera, mesh, device.** A Camera
  (position + target), a Mesh (a name, a list of vertices, a
  position in the world, a rotation), and a Device (the back
  buffer and the render loop). At the end of Part 1, the 8
  points of a cube can be projected and moved in the world.
- **Part 2 — Lines and triangles: wireframe.** Draw lines
  between projected vertices, then triangles, to get a
  wireframe rendering. The line drawing starts with a simple
  recursive midpoint approach and is then replaced by
  Bresenham's line algorithm.
- **Part 3 — Blender meshes in JSON.** Load a mesh exported
  from Blender in JSON format: vertices, faces, normals, and
  texture coordinates. The demo meshes are the Suzanne monkey
  and a torus; the cube from Part 1 remains the hand-built
  test case.
- **Part 4 — Filled triangles: rasterization + Z-Buffer.**
  Fill each projected triangle with scanline rasterization,
  and resolve which triangle is in front with a per-pixel
  depth (Z) buffer.
- **Part 4b — Bonus: parallelism (C# only).** Speed the
  rasterizer with parallel work. This is a performance bonus,
  not a correctness feature, and it is not part of the
  JavaScript/TypeScript engine (see §6).
- **Part 5 — Flat Shading & Gouraud Shading.** Add a diffuse
  point light and shade triangles flat, then smoothly
  (Gouraud) across each triangle.
- **Part 6 — Textures, back-face culling and WebGL.** Map a
  texture onto triangles, skip triangles facing away from
  the camera, and compare the result with the same mesh in
  the existing Babylon.js WebGL engine.

## 3. Camera and matrices

- Camera: a position in the world and a target it looks at.
  Both are 3D vectors.
- A mesh is first centred on itself, then moved into the
  world by translation, scaling, or rotation via matrices,
  then viewed by the camera, then projected into 2D screen
  space. The pipeline is: world matrix × view matrix ×
  projection matrix, applied to each vertex.
- Values as taught: field of view 0.78, near plane 0.01,
  far plane 1.0.
- Matrix theory is **not** taught in the tutorial: the reader
  is sent to external articles on world, view, and projection
  matrices and is told to treat matrix operations as a black
  box supplied by a library (SharpDX for C#,
  `babylon.math` for TypeScript/JavaScript, and their modern
  equivalents under §8). These docs do not claim otherwise,
  and the code must not depend on matrix behaviour that the
  tutorial never states. Any matrix operation the code needs
  beyond "multiply these three matrices and transform a
  vertex" is to be specified in DESIGN before it is used.

## 4. Back buffer and render loop

- The engine renders a complete scene once per frame, aiming
  for 60 frames per second.
- The back buffer is a 2D array mapping the screen: one
  entry per pixel, in the resolution the engine renders at.
  It does not have to equal the display resolution: rendering
  at a lower resolution (for example 640×480) and letting the
  display stretch it to a larger size is explicitly allowed,
  and is the tutorial's only form of "anti-aliasing" (GPU
  hardware scaling, not an engine feature — see §6).
- C#: the back buffer is a byte array, presented each frame
  through a writable bitmap bound to an image control,
  refreshed on the rendering event.
- TypeScript/JavaScript: the back buffer is the canvas
  pixel buffer, accessed through the canvas image-data
  functions, refreshed by `requestAnimationFrame`.
- The render loop, per frame, in order: clear the back
  buffer and the depth buffer; for each mesh, transform its
  vertices into world, view, and projection space; project
  them to screen space; rasterize and shade its triangles;
  present the buffer.

## 5. Rasterization

- **Wireframe (Part 2).** Connect projected vertices with
  lines. First a recursive midpoint method, then Bresenham's
  algorithm. Wireframe is a stepping stone, not the final
  output: the filled-triangle stage replaces it.
- **Filled triangles (Part 4).** Scanline rasterization:
  sort the triangle's vertices by Y, walk the edges with
  inverse slopes, and fill each scanline between the left
  and right edges. The tutorial's per-scanline routine is
  named `ProcessScanLine`; its job is to step across one
  horizontal line of a triangle and write the pixels on it,
  interpolating every value that varies across the triangle
  (depth, normal, texture coordinates) as it goes.
  Interpolation and clamping helpers are named `Interpolate`
  and `Clamp` in the tutorial; their behaviour is: linearly
  interpolate a value between two endpoints by a gradient,
  and clamp a value into a range.
- **Depth (Z) buffer (Part 4).** A per-pixel array holding
  the depth of the last pixel written at that position. A
  new pixel is written only when its depth passes the stored
  depth (nearer wins); otherwise it is discarded. The exact
  comparison direction and initial depth value are
  **to verify before code** (Part 4 prose was not read in
  full), and must be pinned down by a depth-ordering test
  in TEST_PLAN before the rasterizer is considered done.
- **Clipping (seen).** Screen-space point clipping only:
  points outside the screen are not drawn. No triangle
  clipping (splitting a triangle that crosses the screen
  edge) was seen, and none is claimed.

## 6. Shading, textures, culling, parallelism

- **Lighting model (Part 5).** One diffuse point light only.
  No distance falloff was seen. For a surface point, the
  light contribution is the surface colour multiplied by
  `max(0, cos(angle))`, where the angle is between the
  surface normal and the direction to the light. The
  tutorial's routine for that cosine is named `ComputeNDotL`.
  - Flat shading: one normal per face (the average of the
    face's vertex normals, as taught), one shade per
    triangle.
  - Gouraud shading: a normal per vertex, interpolated
    across the triangle during scanline rasterization, so
    the shade varies smoothly per pixel. The light
    contribution is combined with the interpolated normal
    and the texture colour (see below) at each pixel.
  - Not seen, not claimed: specular or Phong highlights,
    shadows, reflections, refraction, multiple lights.
- **Textures (Part 6).** A Texture object holding a fixed-size
  image buffer. Texture coordinates (UVs) come from the
  Blender JSON mesh, are interpolated across the triangle
  in scanlines, and are wrapped with a modulo operation so
  coordinates outside [0,1] repeat the texture. The sampled
  texel colour is combined with the Gouraud shade at each
  pixel. Texture-interpolation code detail beyond this
  description is **to verify before code**.
- **Back-face culling (Part 6).** Skip a triangle whose
  normal faces away from the camera, decided by the dot
  product of the triangle normal and the vector from the
  camera to the polygon. The tutorial reports a bug in its
  own culling: some triangles are wrongly culled because
  the normal transformation ignores camera perspective, and
  reports a roughly 66% speedup (15 → 25 FPS in IE11) from
  culling. Both facts are recorded here as tutorial claims
  to reproduce or refute in testing, not as guarantees:
  the culling test in TEST_PLAN must check that no
  front-facing triangle disappears, and the speedup claim
  must not be asserted for modern hardware.
- **Parallelism (Part 4b, C# only).** Reported results, as
  taught, not as guarantees: parallelising per scanline made
  the demo slower (45 → 40 FPS); parallelising per triangle
  with a per-pixel lock array reached about 53 FPS.
  HTML5 / Web Workers parallelism was explicitly not
  implemented in the tutorial, and is not part of this
  project. Parallelism is a performance ticket (T6) that
  comes after correctness; a parallel version must pass
  every correctness test the serial version passes, pixel
  for pixel, or it does not ship.
- **WebGL (Part 6, comparison only).** The same JSON mesh is
  loaded into the existing Babylon.js WebGL engine to show
  the same scene rendered by a GPU. The reader does not
  build a WebGL engine, and neither does this project.
  No WebGL code is in scope.

## 7. Default scenes (for testing, not a file format)

- Scene A (hand-built, from Part 1): one cube, 8 vertices,
  12 triangular faces, rotating in the world.
- Scene B (imported, from Part 3): the Suzanne monkey mesh
  from a Blender JSON export, rotating.
- Scene C (imported, from Part 3): a torus mesh, used for
  the texture/shading demos.
- The engine has no scene-file format of its own: scenes
  are code (a list of meshes) plus Blender JSON files for
  imported meshes. A scene-file format would be a separate
  project, not a hidden feature.

## 8. Divergences from the tutorial, stated up front

- **Tooling (the big one).** Tutorial: Windows 8, Visual
  Studio 2012, TypeScript 0.9, SharpDX, `babylon.math`.
  Proposed: a current .NET SDK with a current writable-bitmap
  presentation stack for C#, and a current TypeScript
  compiler with plain HTML5 canvas for TypeScript/JavaScript.
  The algorithm is unchanged; only the host stack changes.
  This switch is **conditional on the toolchain check** in
  TEST_PLAN §0: if a modern equivalent changes observable
  behaviour (pixel values, frame order, coordinate
  handedness), the divergence must be documented and the
  affected tests re-based, not silently adjusted.
- **Language parity.** The tutorial gives C#, TypeScript,
  and JavaScript in parallel. These docs keep all three as
  first-class targets with the same behaviour tests. If the
  toolchain check shows one language cannot be hosted
  cleanly, that language is dropped explicitly in a docs
  revision, not quietly left unimplemented.
- **No new features.** Nothing in §6's "not seen" list is
  added to make the engine "better": no shadows, no Phong,
  no triangle clipping, no Web Workers parallelism, no
  image-file export. Those are follow-up projects, not
  scope creep in this one.

## 9. Non-goals (honest)

Ray tracing, raycasting, model editing, save/load of edited
models, triangle clipping, shadows, reflections, refraction,
specular/Phong shading, multiple lights, light falloff,
anti-aliasing inside the engine, Web Workers parallelism,
and building a WebGL engine. None of these is claimed.
