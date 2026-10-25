# GLOSSARY — PBR Book, Scoped Subset

Terms below are defined as used in this docs set. Chapter
references are per the 4th-edition contents page; definitions
are our working summaries, to be verified against chapter
text before code where marked in SPEC.

- **Physically based rendering.** Rendering by modelling
  the physics of light scattering, rather than by ad-hoc
  shading tricks. The book's approach; this build follows
  it only within the scoped chapters.
- **Path tracing.** Tracing rays from the camera and
  letting them bounce between surfaces, accumulating
  light along each path, to solve the light transport
  equation (Ch. 13).
- **Light transport equation.** The equation describing
  how light arriving at a point relates to light emitted
  and reflected elsewhere in the scene (Ch. 13).
- **Monte Carlo integration.** Estimating an integral by
  averaging random samples (Ch. 2). Used here for
  sampling directions and estimating light transport.
- **Inversion method.** Generating a sample from a
  distribution by inverting its cumulative distribution
  function (Ch. 2).
- **Radiometry.** The measurement of light in physical
  units (Ch. 4). The book's basis for colour and
  brightness; this build's colour model (spectral vs
  RGB) is a code-time decision (see SPEC §2).
- **Spectrum / spectral distribution.** How light energy
  is distributed across wavelengths (Ch. 4). Not to be
  assumed present in this build until the Ch. 4
  decision is recorded.
- **BSDF.** Bidirectional scattering distribution
  function: how a surface scatters incoming light into
  outgoing directions (Ch. 9). The interface used by
  the path tracer: sample a direction, evaluate the
  scattering, and give its probability (PDF).
- **BRDF.** The reflective part of a BSDF (no
  transmission). Used for diffuse, specular, and
  conductor models (Ch. 9).
- **Dielectric.** A non-metal material (glass, plastic,
  water) that both reflects and refracts light (Ch. 9).
  Its BSDF includes transmission, unlike a conductor's.
- **Conductor.** A metal material that reflects light
  with a tint and transmits (essentially) none (Ch. 9).
- **Microfacet theory.** Modelling a rough surface as
  many tiny facets, so roughness spreads reflections
  (Ch. 9).
- **BVH (bounding volume hierarchy).** A tree of
  bounding boxes over scene primitives, used to skip
  whole groups of primitives during ray intersection
  (Ch. 7). Must not change the image beyond tolerance.
- **Primitive.** A shape plus its material, as the unit
  the renderer intersects and shades (Ch. 7).
- **Aggregate.** A collection of primitives treated as
  one intersectable object, e.g. via a BVH (Ch. 7).
- **Interaction.** The data recorded at a ray–surface
  hit: position, normal, material, and related values
  (Ch. 3). Exact fields to verify against chapter text.
- **Camera / film.** The camera generates rays for
  pixels; the film accumulates the samples for each
  pixel into the final image (Ch. 5).
- **Projective camera.** A camera model based on
  perspective projection (Ch. 5). The primary camera
  for this build.
- **Spherical camera.** A camera model using spherical
  projection (Ch. 5). Stretch goal only.
- **Literate programming.** Combining human-readable
  documentation and source code into a single reference
  (Ch. 1). The book's structure; this build's docs
  follow the same spirit (docs first, code mapped to
  chapters) without copying the book's code.
- **PPM.** A simple plain-text image format (header +
  RGB values). Proposed output format for this build
  (see README); not claimed as the book's format.
