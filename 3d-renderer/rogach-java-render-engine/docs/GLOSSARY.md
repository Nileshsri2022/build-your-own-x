# GLOSSARY — Rogach Pure Java 3D Render Engine

- **Vertex.** A point in 3D space, given by three
  coordinates (x, y, z).
- **Triangle.** Three vertices plus a colour. The
  only primitive in this project; every object is a
  list of triangles.
- **Tetrahedron.** The demo object: four triangles
  forming a pyramid with a triangular base, centred
  at the origin, faces coloured white, red, green,
  and blue.
- **Orthographic projection.** Projecting 3D points
  onto the screen by discarding the depth coordinate
  (z). Objects do not shrink with distance, unlike
  perspective projection, which this project does
  not use.
- **Rotation matrix.** A 3×3 matrix that rotates a
  point by matrix multiplication. Heading and pitch
  are each one such rotation; combining them is
  matrix multiplication.
- **Heading / pitch.** The two rotations in this
  project: heading is left–right rotation (the
  bottom slider, 0–360 degrees), pitch is up–down
  rotation (the side slider, −90–90 degrees).
- **Barycentric coordinates.** Three weights
  (b1, b2, b3) that say where a point lies relative
  to a triangle's vertices. A pixel is inside the
  triangle exactly when all three weights lie in
  [0,1]; the same weights interpolate depth across
  the triangle.
- **Rasterization.** Converting a triangle into the
  list of screen pixels it covers, here by testing
  every pixel in the triangle's bounding box with
  barycentric coordinates.
- **Bounding box.** The smallest axis-aligned
  rectangle containing a triangle's projected
  vertices, clamped to the image bounds, used to
  limit which pixels the rasterizer tests.
- **Z-buffer (depth buffer).** A per-pixel array
  storing the depth of the closest surface seen so
  far at that pixel, so nearer triangles win over
  farther ones regardless of list order.
- **Normal.** A unit vector perpendicular to a
  triangle's surface, computed as the cross product
  of two of its edge vectors, normalised. It decides
  how the triangle faces the light.
- **Flat shading.** Shading where every pixel of a
  triangle gets the same shade, computed once from
  the triangle's normal and the light direction.
  Contrast Gouraud shading (per-vertex, interpolated),
  which this project does not do.
- **Directional light.** A light with a direction
  but no position, as if infinitely far away. Here
  it is fixed behind the camera, direction (0,0,1).
- **sRGB.** The colour encoding Java uses, already
  scaled to match human perception. Shading must
  convert to a linear scale, shade, and convert
  back, or the falloff looks wrong. This project
  approximates the conversion with a 2.4 power.
- **Inflating.** The bonus technique for approximating
  a sphere: subdivide each triangle into four, then
  push every vertex outward until it lies on a
  sphere of the chosen radius.
- **Swing.** Java's built-in GUI toolkit, used here
  for the demo window (`JFrame`, `JPanel`, sliders)
  only. The renderer itself does not depend on a
  window and is tested headlessly.
- **BufferedImage.** Java's in-memory image, here
  `TYPE_INT_ARGB`, which the renderer writes pixel
  by pixel and the window draws on screen.
