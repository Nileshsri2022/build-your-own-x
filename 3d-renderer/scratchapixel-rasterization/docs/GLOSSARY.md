# GLOSSARY — Scratchapixel Rasterization

- **Rasterization.** Turning 3D geometry into pixels by
  projecting triangles onto the screen and filling the
  pixels they cover. Object-centric: the outer loop is
  over triangles, the inner loop over pixels.
- **Ray tracing.** The contrasting technique: image-
  centric, one ray per pixel, nearest hit wins. This
  project does not use it.
- **Visibility problem.** Deciding which parts of 3D
  objects are visible to the camera and which are hidden
  (outside the field of view, or obscured by nearer
  objects). Rasterization solves visibility; it does not,
  by itself, solve shading.
- **Frame buffer.** A 2D array of pixels holding the
  image being built, one colour per pixel.
- **Z-buffer (depth buffer).** A 2D array of depth values,
  one per pixel, initialised to infinity. When a new
  triangle covers a pixel, its depth is compared with
  the stored value; the nearer one wins, and the stored
  depth is updated.
- **Perspective projection.** Mapping 3D camera-space
  coordinates to 2D screen coordinates so that nearer
  objects appear larger, by dividing by depth
  (perspective divide).
- **NDC (normalized device coordinates).** Screen
  coordinates rescaled to [-1, 1] in both axes, the GPU
  convention used by the lesson, before conversion to
  pixel (raster) coordinates.
- **Raster space.** Pixel coordinates: the image’s own
  grid, where a pixel is addressed by its column and row.
- **Edge function.** A signed function for one edge of a
  projected triangle (Pineda 1988): positive on one side
  of the edge, negative on the other, zero on the edge.
  A pixel is inside the triangle when it is on the
  inside side of all three edges.
- **Barycentric coordinates.** Three weights, one per
  triangle vertex, summing to 1, giving a point’s
  position inside a triangle as a weighted average of
  the vertices. Used to interpolate vertex attributes
  (colour, st coordinates, depth) across the triangle.
- **Top-left rule.** The tie-breaking rule for pixels
  exactly on a shared edge: such a pixel belongs to
  exactly one of the two adjacent triangles, so shared
  edges are drawn once, with no gaps and no double
  drawing.
- **Bounding box.** The smallest axis-aligned rectangle
  containing a projected triangle. Iterating only over
  its pixels, instead of the whole image, is the
  lesson’s main optimisation over the naive loop.
- **Perspective-correct interpolation.** Interpolating
  an attribute (e.g. a texture coordinate) in a way
  that accounts for perspective, rather than linearly
  in screen space, which gives the wrong result for
  surfaces at an angle to the camera.
- **PPM.** The output image format here: a simple,
  uncompressed image format written directly by the
  program, with no image library. Chosen by the lesson
  because there is no cross-platform way to display an
  image directly from a plain C++ program.
- **Near plane / near clipping.** The closest distance
  at which geometry is valid. Geometry crossing the
  near plane must be clipped (or handled) before
  projection, or it produces invalid coordinates.
- **Fixed-point coordinates.** Representing coordinates
  with a fixed number of fractional bits (the lesson’s
  concluding chapter discusses 4 bits of sub-pixel
  precision), as an optimisation over floating point.
  Not required for the first working build.
- **Sub-pixel sampling.** Anti-aliasing by taking
  multiple samples inside each pixel and averaging
  them. Discussed in the lesson’s concluding chapter
  and set as an exercise; not shipped code in this
  project.
