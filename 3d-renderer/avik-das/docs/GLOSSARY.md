# GLOSSARY — 3D Renderer

- **Backward ray tracing.** Casting rays from the camera into
  the scene, instead of from lights outward. Only rays that
  can reach the camera are simulated.
- **Image plane.** The rectangle in front of the camera that
  the image is projected onto; defined by its four corners.
  Each pixel corresponds to a region of this plane.
- **Bilinear interpolation.** Finding a point on the image
  plane by lerping horizontally, then lerping the results
  vertically (see SPEC lesson 1).
- **Lerp.** Linear interpolation: `(1-t)*a + t*b`.
- **Ray.** An origin point plus a direction vector; a point
  on the ray is `origin + t * direction` for `t >= 0`.
- **Intersection test.** Solving whether (and where) a ray
  hits an object — here, the sphere quadratic in SPEC.
- **Normal.** A unit vector perpendicular to the surface at
  a hit point; for a sphere, `(hit - centre) / radius`.
- **Phong shading.** Ambient + diffuse + specular lighting
  model (see SPEC lesson 3). Not physically based; it is an
  approximation that looks right for shiny surfaces.
- **Diffuse.** Light scattered equally in all directions;
  strength depends on the angle between normal and light
  (`N.L`).
- **Specular.** A shiny highlight; strength depends on how
  close the view direction is to the reflected light
  direction, raised to the shininess power.
- **Shadow ray.** A ray from a hit point toward a light, used
  only to test whether anything blocks that light.
- **Reflection / reflectivity.** `k` in `[0,1]`: how much of
  a surface's colour comes from a recursively traced
  reflected ray, vs its own Phong colour.
- **Distributed ray tracing / anti-aliasing.** Taking
  multiple jittered samples per pixel and averaging them,
  to soften jagged edges.
- **Epsilon.** A tiny offset (`1e-4`) used to avoid a ray
  immediately re-hitting the surface it just left.
- **PNG.** The output image format here, written by Java
  `ImageIO`; lossless, 8 bits per channel.
