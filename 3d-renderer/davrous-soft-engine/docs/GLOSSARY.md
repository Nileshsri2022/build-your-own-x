# GLOSSARY — 3D Soft Engine (Davrous)

- **Soft engine / software engine.** A 3D engine that does
  all its drawing on the CPU, with no GPU rendering API.
  "Soft" here means software-only, not "easy".
- **Back buffer.** A 2D pixel array the engine draws into
  each frame, presented to the screen only when the frame
  is complete. One entry per pixel at the render
  resolution, which may be lower than the display size.
- **Render loop.** The per-frame cycle: clear buffers,
  transform and project every mesh, rasterize and shade
  every triangle, present the back buffer. Target here:
  60 frames per second.
- **Camera.** A position in the world plus a target it
  looks at. No other camera controls are in scope.
- **Mesh.** A 3D object described by a list of vertices
  and a list of faces (triangles), plus a position and a
  rotation in the world. Here: a hand-built cube, or a
  Blender JSON export (Suzanne, torus).
- **World / view / projection matrices.** Three matrices
  applied in order to a vertex: world (place the mesh in
  the world), view (place the world relative to the
  camera), projection (turn the 3D view into 2D screen
  space, with perspective). Treated as a black box in the
  tutorial, supplied by a matrix library — see SPEC §3.
- **Field of view (FOV).** How wide the camera sees, as
  taught: 0.78. Near plane 0.01, far plane 1.0.
- **Wireframe.** Drawing only the edges of triangles, as
  lines, with no filled faces. A stepping stone in Part 2.
- **Bresenham's line algorithm.** An integer-step method
  for drawing a straight line on a pixel grid with no gaps.
  The shipped line method in Part 2, replacing an earlier
  recursive midpoint method.
- **Rasterization.** Turning a projected triangle into
  pixels: deciding which pixels it covers and filling them.
  Not ray tracing — no ray is cast per pixel.
- **Scanline rasterization.** Filling a triangle one
  horizontal line at a time, walking its edges with inverse
  slopes and interpolating depth, normals, and texture
  coordinates across each line.
- **Z-Buffer / depth buffer.** A per-pixel array holding
  the depth of the last pixel written there. A new pixel
  is written only if it is nearer than the stored depth.
- **Flat shading.** One shade per triangle, from one face
  normal (the average of the face's vertex normals).
- **Gouraud shading.** Smooth shading: a normal per
  vertex, interpolated across the triangle, so the shade
  varies per pixel instead of per face.
- **Diffuse light.** Light scattered by a matte surface.
  Here: a single point light, no distance falloff, shade =
  colour × max(0, cos(angle between normal and light
  direction)).
- **Texture / UV.** An image mapped onto a triangle.
  UVs are the texture coordinates per vertex, interpolated
  across the triangle; wrapping (modulo) repeats the
  texture outside [0,1].
- **Back-face culling.** Skipping triangles whose normal
  faces away from the camera, decided by a dot product.
  The tutorial reports a bug in its own culling — see
  SPEC §6 and TEST_PLAN.
- **Hardware scaling.** Letting the GPU stretch a
  low-resolution back buffer to the display size. The
  tutorial's only form of "anti-aliasing"; not an engine
  feature, and not claimed as one here.
- **Babylon.js / WebGL (comparison only).** An existing
  GPU engine used in Part 6 to render the same JSON mesh
  for comparison. Not built by this project.
