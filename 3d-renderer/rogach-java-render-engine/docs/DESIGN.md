# DESIGN — Rogach Pure Java 3D Render Engine

## Layout

```
pom.xml
src/main/java/com/rogach/
  Vertex.java        # immutable 3D point: x, y, z
  Triangle.java      # three vertices + java.awt.Color
  Matrix3.java       # 3x3 matrix: multiply, transform(Vertex)
  Renderer.java      # rasterize(list of triangles, heading, pitch, width, height) -> BufferedImage
  Shade.java         # flat shading: normal, shade coefficient, sRGB<->linear conversion (getShade)
  Inflate.java       # subdivide + inflate a triangle list into a sphere approximation
  DemoViewer.java    # Swing window: JFrame, sliders, panel that draws the rendered image
  PngWriter.java     # write a BufferedImage to a .png path (thin ImageIO wrapper, for tests; ours, not the article's)
  Main.java          # entry point: builds the tetrahedron, opens DemoViewer (and, with an argument, saves a PNG)
src/test/java/com/rogach/
  Matrix3Test.java, RasterTest.java, ShadeTest.java, InflateTest.java, RendererTest.java
```

Package is `com.rogach`, not the article's (the article's
code has no package statement in the excerpts read);
class names follow the article's concepts, which is
what matters for readability. All math in `double`.

## Rendering pipeline

```
render(triangles, headingDeg, pitchDeg, width, height):
  image = new BufferedImage(width, height, TYPE_INT_ARGB)
  zBuffer = array[width * height], filled with NEGATIVE_INFINITY
  transform = headingRotation(headingDeg) * pitchRotation(pitchDeg)   # one matrix, computed once
  for each triangle t:
    v1, v2, v3 = transform applied to t's vertices
    # translate to screen centre (translation is NOT part of the 3x3 matrix)
    shift each vertex by (width / 2, height / 2)
    normal = normalize(cross(v2 - v1, v3 - v1))   # computed from transformed vertices
    shade = Shade.getShade(t.color, |normal.z|)   # flat shading, sRGB-aware (see SPEC)
    # bounding box, clamped to the image
    minX..maxX, minY..maxY from the three vertices, clamped to [0, width-1] / [0, height-1]
    area = (v1.y - v3.y) * (v2.x - v3.x) + (v2.y - v3.y) * (v3.x - v1.x)
    for y in minY..maxY:
      for x in minX..maxX:
        b1, b2, b3 = barycentric weights of (x, y) in the triangle
        if any weight < 0 or > 1: skip   # pixel outside the triangle
        depth = b1 * v1.z + b2 * v2.z + b3 * v3.z   # barycentric-interpolated depth
        idx = y * width + x
        if zBuffer[idx] < depth:
          image.setRGB(x, y, shade.getRGB())
          zBuffer[idx] = depth
  return image
```

Notes on the pipeline, each of which a reviewer should
be able to check against SPEC:

- The transform is computed once per frame, not once
  per vertex and not once per triangle. That is the
  article's stated optimisation, and a test may assert
  it by construction (the pipeline above) rather than
  by timing.
- Shading is per triangle, not per pixel: one shade
  value is computed before the pixel loop, and every
  pixel of that triangle gets it. A per-pixel gradient
  inside a triangle would be Gouraud shading, which
  is out of scope.
- The normal is computed from transformed vertices,
  so it rotates with the object. Computing it from
  untransformed vertices would shade a rotated object
  as if it had not rotated — a test in TEST_PLAN
  catches exactly this.
- Degenerate triangles (area = 0, e.g. all three
  vertices collinear or identical) must not divide by
  zero: they are skipped. This is a stated edge case,
  not a silent NaN in the image.
- The z-buffer comparison uses "greater depth wins"
  because positive z is toward the observer (see
  SPEC §3). Reversing the comparison would paint the
  farthest triangle on top; the depth-ordering test
  catches it.

## Window vs headless rendering

- `DemoViewer` is the only Swing code: frame, panel,
  two sliders, repaint on slider change. It calls
  `Renderer.render` and draws the returned image.
- `Renderer.render` itself touches no Swing, no window,
  and no screen: it returns a `BufferedImage`. That is
  what makes the TEST_PLAN's headless pixel tests
  possible — tests call the renderer directly and
  inspect pixels, with no window ever opened.
- `PngWriter` (ours, not the article's) writes that
  same image with `ImageIO.write(image, "png", file)`.
  No gamma correction or tone mapping beyond the
  shading step in SPEC; the PNG is a byte-level copy
  of the rendered pixels.

## Inflate (sphere approximation)

```
inflate(triangles):
  result = []
  for each triangle t:
    m1 = midpoint(v1, v2); m2 = midpoint(v2, v3); m3 = midpoint(v3, v1)
    result += (v1, m1, m3), (v2, m1, m2), (v3, m2, m3), (m1, m2, m3)   # four triangles, colours inherited
  for each vertex v in result:
    scale v so its length equals the sphere radius (divide by its length, times radius)
  return result
```

- One `inflate` call quadruples the triangle count
  (4 triangles become 16, etc.). Callers choose the
  number of rounds; the default demo in the article
  inflates the tetrahedron, and tests use 0, 1, and 2
  rounds so counts stay small and checkable.
- After inflating, every vertex must lie on the sphere
  (within floating-point tolerance); a test asserts
  that, not just the triangle count.
- Colour is inherited per original triangle, so an
  inflated sphere keeps the four face colours in
  patches — it is not re-coloured smoothly.

## Failure modes

- Empty triangle list: renders a blank (all-transparent
  or all-background) image of the requested size — that
  is correct, not an error.
- Degenerate triangle: skipped, as above; it must not
  corrupt neighbouring pixels or the z-buffer.
- Non-positive width/height: rejected by the renderer
  (illegal argument), not rendered as a 0-size image.
- A PNG write failure (e.g. directory does not exist):
  the exception is reported and the process exits
  non-zero; a partially written PNG, if any, is not
  presented as a success.
