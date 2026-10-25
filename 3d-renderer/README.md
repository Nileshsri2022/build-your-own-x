# 3D Renderer — Build Your Own 3D Renderer

Status: **docs only, awaiting your agreement to develop.
No code, no tests, no code push yet.**

A ray-tracing 3D renderer in **Java 11 + Maven**, built from the
Build Your Own X entry for 3D Renderer:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `3D Renderer`)
- Tutorial followed: _Build your own 3D renderer_ by Avik Das —
  https://avik-das.github.io/build-your-own-raytracer/
  (lessons 0–6, with a project per lesson; Java and JavaScript
  implementations exist — this build follows the Java one)
- Tutorial's Java implementation (for reference, not copied):
  https://github.com/avik-das/build-your-own-raytracer-java
  (outputs a `.png` file, tagged `before-project-N` milestones)

This is a separate build from the earlier CodeCrafters-series
raytracer in `~/workspace/raytracer/` (2026-09-23, numpy/Python).
That code is not reused here; this project follows the Avik Das
lessons, in Java, from scratch.

## What it will be

- Backward ray tracing: one ray per pixel (plus jittered samples
  for anti-aliasing), from the camera through the image plane
- Spheres only, by intersection test (the tutorial's scope)
- Phong shading: ambient + diffuse + specular highlights
- Hard shadows: a shadow ray from the hit point to each light
- Recursive reflections, to a fixed depth, with per-material
  reflectivity
- Distributed ray tracing / anti-aliasing: multiple jittered
  samples per pixel, averaged
- Output: a `.png` file (Java `BufferedImage` + `ImageIO`, no
  external dependencies, no GUI window needed)

## Planned run (not working yet — code is not written)

```sh
mvn clean package
# or, in this VM, javac --release 11 (see distributed-systems README
# for why Maven Central is unreachable from this VM's Java)

java -cp target/renderer-1.0-SNAPSHOT.jar com.renderer.Main out.png
java -cp target/renderer-1.0-SNAPSHOT.jar com.renderer.Main out.png 400 300 4
# args: <output.png> [width height samples-per-pixel]
```

Default scene (ours, defined in SPEC): camera at (0,0,0), image
plane at z=2, three spheres (red centre, green left, blue right),
one white point light, black background.

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — lessons, math, scene, and every deliberate
  divergence from the tutorial
- `docs/DESIGN.md` — classes, rendering loop, file format
- `docs/TEST_PLAN.md` — the tests that must pass before any push
- `docs/TICKETS.md` — build order, following the tutorial's
  lessons, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Divergences from the tutorial, stated up front

- Build tool: Maven, not the tutorial Java repo's Gradle. The
  language (Java) and the PNG output are the same; only the build
  tool changes, so the code stays buildable with the Maven setup
  already used for distributed-systems.
- Language choice: Java, not JavaScript + Canvas 2D. PNG output is
  verifiable headlessly (file exists, dimensions, pixel colours);
  a Canvas version would need a browser to see anything. If you
  want the JavaScript/Canvas version instead, say so before code —
  the switch is docs-only at this stage.
- Scene: the default scene in SPEC is ours (positions/colours
  above), not a claim to reproduce the tutorial repo's exact scene
  file, which was not read for these docs.

## Out of scope, honestly

Triangles/meshes, textures, refraction, soft shadows, global
illumination, GPU/OpenGL, real-time preview, and animation. Those
are not claimed; the tutorial's scope is spheres + Phong +
shadows + reflections + anti-aliasing, and so is this build's.

## Next step

Your agreement on these docs triggers, in order: code (TDD, one
ticket at a time), test run (including a rendered PNG checked by
pixel), code review against SPEC, then push — this project only,
as its own `3d-renderer/` directory in `build-your-own-x`.
