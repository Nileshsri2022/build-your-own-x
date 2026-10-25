# GLOSSARY — Scratchapixel Introduction to Ray Tracing

- **Backward ray tracing.** Casting rays from the eye/camera
  into the scene, instead of from lights outward. Only rays
  that can reach the eye are simulated.
- **Image plane.** The rectangle in front of the eye that the
  image is projected onto; each pixel corresponds to a
  region of this plane. Conceptually, a cut through the
  pyramid whose apex is the eye.
- **Perspective projection.** Translating 3D points onto a
  2D plane by drawing lines from each point to the eye and
  marking where they cross the image plane, creating the
  illusion of depth.
- **Primary ray.** A ray cast from the eye through a pixel
  (here, through the pixel's centre) into the scene.
- **Ray.** An origin point plus a direction vector; a point
  on the ray is `origin + t * direction` for `t >= 0`.
- **Intersection test.** Solving whether (and where) a ray
  hits an object — here, the sphere quadratic in SPEC.
- **Normal.** A unit vector perpendicular to the surface at
  a hit point; for a sphere, `(hit - centre) / radius`.
- **Shadow ray.** A ray from a hit point toward a light, used
  only to test whether anything blocks that light.
- **Conductor / dielectric.** The tutorial's two broad
  material families: conductors are metals; dielectrics
  are non-metals (glass, plastic, wood, water), differing
  in transparency.
- **Reflection.** Light bouncing off a surface along the
  mirror direction about the normal.
- **Refraction.** Light bending as it passes into a
  transparent material such as glass.
- **Fresnel / facing ratio.** The term deciding, for a glass
  surface, how much light reflects vs refracts: head-on
  rays refract more, grazing rays reflect more.
- **Epsilon.** A tiny offset (`1e-4` in DESIGN) used to avoid
  a ray immediately re-hitting the surface it just left.
- **PPM.** The output image format here (ASCII `P3` in the
  tutorial's examples): a text header (`P3`, width, height,
  max value `255`) followed by decimal RGB triples. No
  image library is needed to write it.
