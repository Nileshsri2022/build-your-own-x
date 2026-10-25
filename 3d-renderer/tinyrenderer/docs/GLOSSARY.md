# GLOSSARY — tinyrenderer

- **Software renderer.** A renderer that computes
  every pixel on the CPU and writes an image file,
  without using a GPU or a graphics API.
- **Rasterization.** Turning triangles into pixels:
  project the triangle's vertices onto the screen,
  then fill the pixels its projected shape covers.
  The opposite order from ray tracing, which starts
  from pixels and casts rays into the scene.
- **Triangulated mesh.** A 3D model made only of
  triangles, as stored in a `.obj` file: a list of
  vertices, texture coordinates, and faces (each
  face is three vertex indices).
- **Framebuffer.** The 2D array of colours, one per
  output pixel, that is written to the TGA file at
  the end of a render.
- **Depth buffer / z-buffer.** A 2D array of depths,
  one per pixel, holding the depth of the nearest
  surface seen so far. A farther fragment does not
  overwrite a nearer pixel.
- **Barycentric coordinates.** Three weights, one
  per triangle vertex, that say where a pixel lies
  relative to the triangle (inside if all three are
  between 0 and 1). They are also used to interpolate
  depth, texture coordinates, and normals across the
  triangle.
- **Back-face culling.** Discarding a triangle that
  faces away from the camera, judged here by the
  sign of its projected signed area, before filling it.
- **Phong shading.** A local lighting model:
  ambient + diffuse + specular, where the specular
  term depends on how closely the view direction
  matches the reflected light direction.
- **Tangent space.** The local coordinate system at
  a surface point, built from the tangent, bitangent,
  and normal vectors. A normal map stores normals in
  this space, so they must be transformed into it
  (or out of it) before shading.
- **Normal mapping.** Perturbing a surface's normal
  per pixel using a normal-map texture, so flat
  geometry looks detailed without adding triangles.
- **Shadow mapping.** A two-pass shadow technique:
  first render depth from the light's viewpoint, then
  render from the camera and mark a pixel as shadowed
  if its depth from the light is greater than the
  stored depth. Here the mask is hard and binary.
- **TGA.** The output image format here (Truevision
  Targa), written by the supplied TGA-image class;
  supports RGB, RGBA, and grayscale.
- **`.obj`.** The input mesh format here: a text
  format listing vertices, texture coordinates, and
  (here, triangulated) faces.
