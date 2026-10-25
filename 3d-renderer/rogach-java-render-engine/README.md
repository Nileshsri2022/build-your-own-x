# Rogach — Pure Java 3D Render Engine

Status: **Code built and tested 2026-10-25: Java 11, 22/22 tests + integration PASS (Swing window manual check still open).**

A small software 3D render engine in **Java 11 + Maven**,
built from the Build Your Own X entry for 3D Renderer:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _How to create your own simple 3D render
  engine in pure Java_ by Rogach —
  http://blog.rogach.org/2015/08/how-to-create-your-own-simple-3d-render.html
  (a single article, not a lesson series)

This is one of eleven separate docs sets under `3d-renderer/`,
one per BYOX 3D Renderer link. Each set is its own project:
this one follows Rogach only, and nothing from the other ten
sets is reused or assumed here.

## What it will be

- Triangles as the only primitive, rendered by software
  rasterization on the CPU — no OpenGL, no GPU, no external
  libraries beyond the Java APIs themselves
- Orthographic projection: the depth coordinate is discarded
  for drawing, so objects do not shrink with distance
- Rotation by two sliders: heading (left–right) and pitch
  (up–down), applied as 3×3 matrix transforms to the object
- Triangle rasterization by barycentric coordinates, over
  each triangle's bounding box, writing directly to image
  pixels
- A z-buffer (depth buffer): for each pixel, only the
  closest triangle's colour survives
- Flat shading: one shade per triangle, from the angle
  between the triangle's normal and a fixed directional
  light, with an sRGB/linear approximation so the falloff
  looks right
- A bonus: a sphere approximated by repeatedly subdividing
  each triangle into four and pushing the new vertices out
  to a sphere radius ("inflating")

## Planned run (not working yet — code is not written)

```sh
mvn clean package
# or, in this VM, javac --release 11 (see the distributed-systems
# README for why Maven Central is unreachable from this VM's Java)

java -cp target/rogach-render-engine-1.0-SNAPSHOT.jar com.rogach.Main
# opens a Swing window: a rotating tetrahedron, heading slider
# at the bottom, pitch slider at the side
```

The demo object is a tetrahedron made of four triangles
(white, red, green, blue), centred at the origin.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — article sections, math, scene, and every
  deliberate divergence from the article
- `docs/DESIGN.md` — classes, rendering pipeline, window
  and headless rendering
- `docs/TEST_PLAN.md` — the tests that must pass before any
  push
- `docs/TICKETS.md` — build order, following the article's
  sections, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Divergences from the article, stated up front

- Build tool: Maven, not the article's plain compilation.
  The article states no build tool; it compiles and runs
  plain Java with Swing. The language (Java) and the
  zero-dependency stance are the same; only the build tool
  changes, so the code stays buildable with the Maven setup
  already used for the other Java BYOX docs.
- Test output: the article's output is a live Swing window
  only, with no image-file output stated. These docs add,
  clearly marked as ours, an optional PNG save of the
  rendered image (from the same `BufferedImage` the window
  draws), so tests can check pixels headlessly without a
  window. The window remains the primary output; the PNG
  is a testing aid, not a change to the rendering itself.
- The article's full-source gist link was not captured
  (the article shows it only as "here"), so no gist URL is
  cited anywhere in these docs. The article's claim of
  about 200–220 lines for the whole engine is reported as
  the article's claim, not as a verified count.

## Out of scope, honestly

Perspective projection, Phong or Gouraud shading,
specular highlights, shadows, reflections, refraction,
textures, anti-aliasing, mesh loading, true sphere
primitives, and a camera class. The article explicitly
excludes textures and heavier lighting, and this build
does the same. The "sphere" in this project is a
subdivided tetrahedron, not a sphere primitive — see SPEC.

## Next step

Approval triggers, in order: code (TDD, one ticket at a
time), test run (including headless pixel checks), code
review against SPEC, then a separate push approval for
this folder only.
