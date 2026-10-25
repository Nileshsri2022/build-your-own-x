# GLOSSARY — Ray Tracing in One Weekend (Book 1)

- **Path tracer.** A renderer that computes colour by
  bouncing rays randomly according to material
  scattering, rather than by direct lighting from
  explicit lights. Book 1's renderer is, in the
  book's own words, technically a path tracer.
- **Ray.** An origin point plus a direction vector;
  a point on the ray is `origin + t * direction`.
- **Hittable.** Any object a ray can hit. In Book 1,
  the only hittable is a sphere, plus a list of
  hittables that returns the nearest hit.
- **Hit record.** The data recorded at a hit: hit
  point, surface normal, hit parameter `t`, whether
  the hit was on the front face, and the material hit.
- **Front face / back face.** A hit is on the front
  face when the ray strikes the outward-facing side
  of the surface. The record's normal is flipped to
  face against the ray when the hit is on the back
  face, so shading and refraction work from inside
  as well as outside (needed for hollow glass).
- **Interval.** An accepted range [min, max] for the
  hit parameter `t`. Book 1 accepts hits from 0.001
  upward, to avoid shadow acne.
- **Shadow acne.** Black speckles caused by a
  scattered ray immediately re-hitting the surface
  it just left, due to floating-point error. Fixed
  here by the 0.001 minimum hit distance — not by
  shadows, which Book 1 does not have.
- **Lambertian (diffuse) material.** A matte material:
  it scatters a ray in a random direction derived
  from the surface normal and attenuates colour by
  its albedo. No shiny highlight.
- **Metal material.** A mirror-like material: it
  reflects the incoming ray about the surface normal.
  Fuzzy metal perturbs the reflected direction by a
  random vector scaled by a fuzz factor.
- **Dielectric material.** Glass-like: it refracts
  rays according to Snell's law, reflects totally
  when a ray inside cannot escape, and otherwise
  chooses reflection vs refraction by the Schlick
  approximation.
- **Schlick approximation.** An approximation of the
  Fresnel reflectance of a dielectric surface — the
  probability that light reflects rather than
  refracts — as a function of the cosine of the
  incident angle and the refractive index.
- **Total internal reflection.** When a ray inside a
  denser material hits the surface at too shallow
  an angle, it cannot refract out and must reflect
  instead.
- **Anti-aliasing (supersampling).** Taking several
  jittered samples per pixel and averaging them, so
  jagged sphere edges soften.
- **Sample square.** The unit square around a pixel
  centre from which jittered sample offsets are drawn
  (offsets in [-0.5, +0.5] per axis).
- **Gamma correction.** Adjusting linear light values
  when writing them out, so they display with correct
  brightness on a screen. Listed in Chapter 9's TOC;
  verify its exact form against the book before code.
- **Defocus blur.** Blur from a thin-lens camera:
  rays start from random points on a lens disk, so
  only objects at the focus distance are sharp.
- **PPM (P3).** A plain-text image format: header
  `P3`, width and height, max colour value `255`,
  then RGB integer triples. Book 1's output format,
  written to stdout.
- **Albedo.** The fraction of light a material
  reflects, per colour channel — the attenuation
  applied on each scatter.
