# SPEC — Rogach Pure Java 3D Render Engine

## 1. Idea

Modern 3D engines look like black magic, but a basic one
is not out of reach: take a few triangles, rotate them,
project them onto the screen, fill the pixels they cover,
hide the ones behind other triangles, and shade each
triangle by how it faces the light. Do all of that in
plain Java, with no external libraries, and the result
fits in a couple of hundred lines and runs almost
anywhere. That is the whole project.

## 2. Article sections, in order

- **GUI wrapper.** A Swing window: a frame with a panel
  that displays the rendered image, a slider at the
  bottom for horizontal rotation (heading, 0–360 degrees)
  and a slider at the side for vertical rotation (pitch,
  −90 to 90 degrees). Moving a slider forces a redraw.
  This wrapper is scaffolding, not rendering; the
  rendering code lives in the panel's paint step.
- **Vertices and triangles.** A vertex is three
  coordinates (x, y, z). A triangle binds three vertices
  and a colour. All geometry in this project is a list
  of such triangles; there is no other primitive.
- **Demo object: tetrahedron.** Four triangles describe
  a tetrahedron centred at the origin (0,0,0): the
  colours are white, red, green, and blue, one per face.
  Centring at the origin matters: rotation is then
  rotation about the object's own centre, not about a
  distant point.
- **Wireframe via orthographic projection.** For the
  first visible result, ignore rotation's depth effect
  and ignore filling: translate the origin to the centre
  of the drawing area, discard the z coordinate of each
  transformed vertex, and draw the triangle outlines.
  This is orthographic projection in its simplest form.
- **Rotation matrices.** A 3×3 matrix transforms a point
  by matrix multiplication. Three primitive rotations
  are used: rotation in the XY plane, in the YZ plane,
  and in the XZ plane. Heading and pitch are each one
  such rotation; combining them is matrix multiplication
  of the two rotation matrices into a single transform,
  computed once per frame, then applied to every vertex.
  A 3×3 matrix cannot express translation — that is
  why translation to the screen centre is done separately
  (see above), not by the matrix.
- **Triangle rasterization via barycentric coordinates.**
  For each triangle, compute the rectangular bounding
  box of its projected vertices, clamped to the image
  bounds, and test each pixel in that box: a pixel is
  inside the triangle exactly when all three of its
  barycentric weights lie in [0,1]. Inside pixels get
  the triangle's colour, written directly to image
  pixels (not through the 2D drawing API). Pixels
  outside are skipped. This is deliberately simple and
  not hardware-fast; real engines use hardware
  rasterization, which is out of scope here.
- **Z-buffer (depth buffer).** Painting triangles one
  after another in list order is wrong: the last painted
  triangle wins, even if it is behind another. Fix it
  with a per-pixel depth array, initialised to negative
  infinity (nothing seen yet, infinitely far away).
  For each rasterized pixel, compute its depth by
  barycentric interpolation of the three vertices'
  depths, and write the colour only if that depth is
  greater than (closer than) the stored depth, updating
  the stored depth as well. "Greater" here follows the
  article's convention: depth increases toward the
  viewer (positive z means toward the observer), so
  the closest surface has the largest depth value.
- **Flat shading.** One shade per triangle, from the
  angle between the triangle's surface and the light.
  Steps: compute the normal as the cross product of two
  edge vectors of the triangle, normalised to unit
  length; the light is directional, positioned behind
  the camera at an infinite distance, so its direction
  is the fixed vector (0,0,1); the shade coefficient is
  the absolute value of the normal's z component (the
  cosine of the angle, with the sign dropped because
  this simple build does not distinguish front from
  back faces); multiply the triangle's colour by that
  coefficient. A naive multiply looks wrong because
  Java colours are sRGB, which is already perceptually
  scaled: so convert each channel from sRGB to a linear
  scale (approximated by raising to the power 2.4),
  apply the shade, and convert back (power 1/2.4).
  The 2.4 exponent is an approximation, not the full
  sRGB specification — the article says so, and these
  docs say so too.
- **Bonus: sphere by subdivision and inflating.** A
  sphere is approximated, not rendered as a primitive:
  repeatedly take each triangle, split it into four
  smaller triangles by joining edge midpoints, and then
  push every vertex outward so it lies on a sphere
  (divide each vertex by its length, scaled to the
  sphere radius). More subdivision rounds give a
  smoother sphere. This is a bonus, not a second
  primitive: the renderer still only knows triangles.

## 3. Coordinates and conventions (fixed by this spec)

- x = left–right on screen, y = up–down on screen,
  z = depth, perpendicular to the screen, positive z
  toward the observer.
- The object is centred at the origin before rotation.
- The drawing area's centre is the screen centre: after
  transformation, vertex x/y are shifted by half the
  width/height before rasterization.
- Default window size in the article's wrapper: 400×400.
  The renderer's image size follows the panel size;
  tests use explicit small sizes (see TEST_PLAN).
- Colour: `java.awt.Color` per triangle; image pixels
  are `BufferedImage.TYPE_INT_ARGB`.

## 4. Output and interface (article + our addition)

- Article output: a Swing window only. The panel draws
  the rendered `BufferedImage` with the 2D graphics
  context. No image-file output is stated in the
  article.
- **Our addition, clearly marked:** a `savePng(path)`
  operation on the renderer, writing the same
  `BufferedImage` via `ImageIO`. It exists so tests
  and reviewers can check pixels without a window.
  It does not change any rendering math, and the
  window remains the primary output.
- Interface: heading slider 0–360 (initial 180),
  pitch slider −90–90 (initial 0). Slider changes
  trigger a repaint; nothing else is interactive.
  There is no file loading, no mesh import, no camera
  movement — rotation sliders are the whole UI.

## 5. Non-goals (honest)

Perspective projection (objects shrinking with
distance), Phong or Gouraud shading, specular
highlights, shadows, reflections, refraction, textures,
anti-aliasing, mesh loading, true sphere primitives,
a camera class, and back-face culling as a distinct
feature (the sign-dropping in flat shading means both
faces are shaded alike). The article explicitly puts
textures and heavier lighting outside its scope, and
these docs do the same. Anything in this list that
appears in a later project must be specced there, not
smuggled in here.
