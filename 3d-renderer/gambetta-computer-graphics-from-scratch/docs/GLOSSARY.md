# GLOSSARY — Computer Graphics from Scratch (Gambetta)

- **Canvas.** The abstract drawing surface, of a fixed
  width and height, on which both renderers draw.
- **PutPixel.** The book's only assumed primitive: set
  one pixel on the canvas to one colour. Everything in
  both renderers is built on it.
- **Raytracer (Part I).** A renderer that works
  image-first: it casts one ray per pixel from the camera
  into the scene and colours the pixel by what the ray
  hits. In this book, spheres only.
- **Rasterizer (Part II).** A renderer that works
  object-first: it projects triangles onto the screen and
  fills the pixels they cover. In this book, triangle
  meshes.
- **Sphere.** Part I's only geometry: a centre point, a
  radius, a colour, a specular exponent, and a
  reflectivity.
- **Triangle.** Part II's only geometry primitive: three
  vertices, usually with colours, normals, and texture
  coordinates attached. Meshes are lists of triangles.
- **Model.** A reusable description of an object: its
  vertex list and triangle list, in its own local space.
- **Instance.** A Model placed in a scene, with its own
  transform (position, rotation, scale). One Model can
  have many Instances.
- **Camera.** The viewpoint: a position and an
  orientation, plus projection parameters. Both renderers
  have a camera, but they use it differently (ray origin
  and direction in Part I; view transform in Part II).
- **Perspective projection.** Converting a 3D point to a
  2D screen point so that farther objects appear smaller.
- **Diffuse reflection.** Light scattered by a surface,
  whose strength depends on the angle between the surface
  normal and the light direction.
- **Specular reflection.** A shiny highlight, whose
  strength depends on how close the view direction is to
  the reflected light direction, raised to a specular
  exponent. A high exponent (e.g. 500) gives a tight
  highlight; a low exponent (e.g. 10) gives a broad one.
- **Ambient light.** Light with no direction and no
  position, contributing a constant amount everywhere, so
  nothing is pitch black.
- **Point light.** A light at a position, shining in all
  directions. In this book, white only, with no distance
  falloff.
- **Directional light.** A light with a direction but no
  position, as if infinitely far away. In this book, white
  only.
- **Shadow test.** Checking whether anything blocks the
  path from a hit point to a light; a blocked light
  contributes nothing for that pixel.
- **Reflectivity (r).** A factor in [0, 1] for a mirror-like
  surface: final colour = local colour × (1 − r) +
  reflected colour × r.
- **Recursion depth.** A limit on how many times a
  reflected ray may itself spawn a reflected ray. Beyond
  it, the reflected contribution is the background colour.
- **Clipping.** Removing the parts of a scene the camera
  cannot see, at instance level (whole object) and
  triangle level (part of an object), so rendering from
  any camera position is safe.
- **Depth buffer (z-buffer).** A per-pixel store of the
  depth of the nearest surface drawn so far; a new pixel
  is written only if its surface is nearer.
- **Back-face culling.** Discarding triangles whose front
  face points away from the camera, so they are never
  drawn.
- **Flat shading.** One colour per triangle, from the
  triangle's normal.
- **Gouraud shading.** Colour interpolated across a
  triangle from its vertices' colours/normals, giving a
  smooth gradient.
- **Phong shading.** Shading evaluated per pixel, using
  interpolated normals, giving highlights that move
  correctly with the light.
- **Texture.** An image "painted" onto triangles to fake
  surface detail, using UV coordinates that travel with
  the vertices.
- **Perspective-correct interpolation.** Interpolating
  attributes (colour, texture coordinates) across a
  triangle using reciprocal depth (1/z), so a receding
  surface looks correct. Linear screen-space
  interpolation is the incorrect contrast, not a
  substitute.
- **UV coordinates.** Two numbers per vertex that say
  where on a texture image that vertex sits.
