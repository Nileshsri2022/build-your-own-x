# SPEC — Scratchapixel Rasterization

## 1. Idea

Render a 3D scene to a 2D image by rasterization. Both raster-
ization and ray tracing solve the visibility problem: which
parts of 3D objects are visible to the camera, and which are
hidden because they are outside the field of view or obscured
by other objects. They differ in loop order:

- Ray tracing (image-centric):
  `for each pixel: trace a ray; keep the closest hit.`
- Rasterization (object-centric):
  `for each triangle: project its vertices; fill the pixels
  inside the projected 2D triangle.`

This project is the second one. Its core claim, from the
lesson, is that the whole algorithm rests on two simple,
fast techniques: perspective projection of vertices, and
deciding whether a pixel lies inside a 2D triangle. That is
why it suits GPUs — but this build is a CPU program, not a
GPU program.

## 2. Chapters (tutorial order)

- **(1) An Overview of the Rasterization Algorithm.**
  Fully read. Naive form: outer loop over triangles, inner
  loop over all pixels, testing each pixel against the
  projected triangle. Optimisation: iterate only over the
  pixels in the projected triangle’s 2D bounding box instead
  of the whole image. Frame buffer: a 2D array of pixels.
  Z-buffer introduced: a depth value per pixel, so that when
  two projected triangles overlap a pixel, the nearer one
  wins.
- **(2) The Projection Stage.** Partially read (first part
  only; the tail was not fetched). Camera space → screen
  space by perspective divide, keeping the vertex z (or its
  negation, per the lesson’s convention — to be confirmed
  against the page before code, see §6). Screen space →
  NDC in [-1, 1], the GPU convention. NDC → raster space
  (pixel coordinates). Near-clipping is handled: geometry
  in front of the near plane must not produce garbage
  projected coordinates.
- **(3) The Rasterization Stage.** Partially read (first
  part only; the tail was not fetched). Edge function
  (Pineda 1988): for each edge of the projected triangle, a
  signed function that is positive on one side, negative on
  the other, zero on the edge. A pixel is inside the
  triangle when it is on the inside side of all three
  edges. Barycentric coordinates of the pixel inside the
  triangle are used to interpolate vertex attributes
  (here: vertex colour, and st/texture coordinates as data).
  Top-left rule: pixels exactly on a shared edge are
  assigned to exactly one of the two adjacent triangles,
  so no pixel is drawn twice and no gap appears.
  The basic rasterizer in this chapter saves its output
  to a PPM file.
- **(4) The Depth Buffer.** REFERENCED BY TITLE ONLY —
  UNVERIFIED. The projection chapter names a "Chapter Four"
  with this title, but its page was not opened and its
  exact URL and contents are not known. Nothing in these
  docs that claims to come from that chapter is fact.
  What IS known, from the overview and concluding chapters,
  is only: a float depth array, initialised to infinity,
  one depth per pixel, nearest triangle wins. Any finer
  detail (exact interpolation of depth, exact initial
  value convention, any edge case) must be verified from
  that chapter before code, or derived and tested as our
  own design decision and labelled as such.
- **(5) Rasterization: a Practical Implementation
  (concluding chapter).** Partially read (first part only;
  the concluding code walkthrough and exercises are known
  only from a search snippet, not from a full fetch — see
  §6). Topics: anti-aliasing by sub-pixel sampling is
  discussed and set as an exercise (e.g. 4 sub-pixels),
  not shipped code. 8×8 pixel blocks: the image is
  processed in blocks for cache behaviour. Incremental
  edge-function evaluation: edge values are updated by
  addition as the scan moves to the next pixel, rather
  than recomputed from scratch. Fixed-point coordinates:
  sub-pixel precision with a fixed number of fractional
  bits (the snippet says 4 bits). Perspective-correct
  interpolation of attributes (as opposed to linear
  interpolation in screen space, which is wrong for
  perspective). The chapter warns: "Use this code for
  learning purposes only. It is far from efficient…
  we prioritized clarity over efficiency." That warning
  is carried into DESIGN and TEST_PLAN: this build is
  judged on correctness, not speed.

## 3. Geometry and input

- Primitives are triangles only. Objects are assumed to be
  collections of triangles. Geometry that is not made of
  triangles must be triangulated before it reaches the
  rasterizer — triangulation itself is out of scope.
- The final code in the lesson reads triangle vertices
  (and texture/st coordinates) from an include file. Our
  build follows that shape: a scene is a list of triangles
  defined in code (or in a simple data file read at
  startup — the exact input mechanism is a DESIGN
  decision, not a lesson requirement, and must be stated
  in DESIGN before code).
- Default scene (ours, for the first pixel-checkable
  render): a single triangle, centred in the image, with
  three different vertex colours, so that coverage,
  barycentric interpolation, and the top-left rule can
  all be checked from one PPM file. Exact coordinates
  are fixed in DESIGN before code.

## 4. Pipeline (what the program does, in order)

1. For each triangle: transform its vertices from camera
   space to screen space (perspective divide, keeping z
   for depth).
2. Convert screen coordinates to NDC [-1, 1], then to
   raster (pixel) coordinates.
3. Compute the triangle’s 2D bounding box, clamped to
   the image bounds.
4. For each pixel in the bounding box: evaluate the three
   edge functions; if the pixel is inside (top-left rule
   applied), compute its barycentric coordinates.
5. Compare the pixel’s depth with the z-buffer value at
   that pixel; if nearer, write the interpolated colour
   and update the z-buffer; otherwise leave the pixel.
6. Write the frame buffer to a PPM file.

## 5. Output and format

- Output is a PPM file (the lesson’s choice, because there
  is no cross-platform way to display an image directly
  from a plain C++ program). The PPM is written by the
  program itself, with no image library.
- PPM layout (to be fixed exactly in DESIGN before code):
  the P3 (ASCII) or P6 (binary) variant, width, height,
  max colour value, then pixel data in row-major order,
  top row first. The choice between P3 and P6 is ours;
  P3 is easier to inspect by eye and to test by text
  comparison, P6 is smaller. Decide in DESIGN, and test
  exactly that variant.
- Colour: 8 bits per channel. Vertex colours are given
  as 8-bit values; interpolated colours are rounded to
  the nearest 8-bit value (rounding rule fixed in DESIGN).
- No window, no GUI, no image-file reading, no PNG/TGA.

## 6. Unverified items — read these before trusting them

1. **Depth Buffer chapter (Chapter 4):** not opened.
   Title only. Verify its URL and contents before code;
   do not spec its internals as fact.
2. **Projection-stage tail:** the sign convention for the
   kept z (z vs -z), and the exact near-clipping
   procedure, were in the part of the chapter that was
   not fetched. Confirm against the page before code.
3. **Rasterization-stage tail:** the exact basic-
   rasterizer code, and any detail beyond the edge
   function / barycentric / top-left rule summary above,
   was not fetched in full. Confirm before code.
4. **Concluding chapter tail:** the final implementation
   code, the frame-buffer/PPM output detail, and the
   exercises are known only from a search snippet.
   Confirm before code; do not quote its code.
5. **C++ toolchain in this VM:** NOT verified. See
   TEST_PLAN §1 — toolchain check is a pre-code step,
   and no "tested" claim may be made until it passes.

## 7. Non-goals (honest)

No lighting model of any kind (no Phong, diffuse, or
specular), no shadows, no reflections, no refraction, no
texture sampling, no anti-aliasing in shipped code, no
ray tracing, no mesh file loading beyond the lesson’s
triangle-vertex input, no animation, no GPU API. The
concluding chapter’s own sentence stands as the scope
statement: "This algorithm offers no shading solutions."
