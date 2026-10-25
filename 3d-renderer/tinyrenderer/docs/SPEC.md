# SPEC — tinyrenderer (software rasterizer)

## 1. Idea

Modern graphics APIs (OpenGL, Vulkan, Metal, DirectX) all do
roughly the same thing: take a triangulated mesh, transform
its vertices into screen space, fill its triangles pixel by
pixel, and shade each pixel. This build re-implements that
pipeline in software, in bare C++, so the pipeline itself —
not the GPU — is what is being learned. The tutorial's own
framing is kept: "I do not intend to show how to write GPU
applications — I want to show how they work."

This is rasterization, not ray tracing. There are no rays
cast from a camera through pixels, no sphere intersections,
and no recursive reflection rays. Visibility is resolved by
projecting triangles and keeping, per pixel, the nearest
surface (the depth buffer). That difference from the Avik
Das raytracer in the parent `3d-renderer/` docs is the
whole point of having a separate build for this link.

## 2. Pipeline (verified features, in render order)

1. **Load.** Read a triangulated `.obj` mesh (vertices,
   texture coordinates, faces) and its texture(s). At the
   start, the only image operation available is setting a
   single pixel; the supplied TGA-image class handles
   reading/writing TGA files (RGB, RGBA, grayscale) and
   nothing else.
2. **Transform.** Transform mesh vertices from model space
   into screen space (the camera/projection step).
   To verify before code: the exact camera model and
   projection used by the course notes (see README).
3. **Cull.** Back-face culling by a signed-area test:
   triangles whose projected signed area shows them facing
   away from the camera are discarded before filling.
4. **Rasterize.** Fill each surviving triangle pixel by
   pixel, using barycentric coordinates to decide whether
   a pixel lies inside the triangle and to interpolate
   per-vertex data (texture coordinates, normals) across
   it.
5. **Depth test.** A depth buffer (z-buffer) keeps, per
   pixel, the nearest surface seen so far; a triangle
   fragment that is farther away than the stored depth
   does not overwrite the pixel.
6. **Shade.** Local shading with Phong named as the model
   (ambient + diffuse + specular, as in the usual Phong
   formulation — exact coefficients are a code-level
   detail to be fixed when the shading ticket is written,
   and must be recorded there, not assumed here).
7. **Normal mapping.** Tangent-space normal mapping:
   build a tangent / bitangent / normal basis from the
   UV grid, perturb the surface normal per pixel from
   the normal map, and shade with the perturbed normal.
8. **Shadow mapping.** Two passes. Pass 1 renders the
   scene from the light's point of view into a depth
   buffer. Pass 2 renders from the camera; a pixel whose
   depth from the light is greater than the stored depth
   is in shadow. The shadow mask is hard and binary
   (in shadow or not), not soft.

## 3. Input and output

- Input: one or more triangulated `.obj` files (the
  tutorial repo's example uses a character-pose model
  plus a floor model), and the texture files those
  models reference.
- Output: a single TGA image file, `framebuffer.tga`
  (the tutorial repo's output name). No window, no
  interactive display, no animation loop.
- CLI (our design, around the tutorial repo's behaviour
  of taking model files as arguments):
  `tinyrenderer <model.obj> [floor.obj] [output.tga]`
  - At least one model file is required.
  - The output argument, if omitted, defaults to
    `framebuffer.tga`.
  - Bad arguments (no model, unreadable file, output
    path not writable) print usage / an error to
    stderr and exit non-zero, with no partial output
    presented as a success.
  - To verify before code: the tutorial's exact
    argument order and defaults; if they differ, this
    section is revised before code, not after.

## 4. Defaults (ours, to be confirmed against the notes)

- Output resolution: fixed at build time by the
  framebuffer the program creates (the tutorial's
  starting-point example uses a small framebuffer;
  the final renderer's size is a code-level constant
  to be fixed in DESIGN and recorded there).
- No anti-aliasing is specified in this build. Any
  anti-aliasing (e.g. supersampling) is out of scope
  unless a verified lesson adds it.
- No gamma correction is specified. Colour values are
  written as computed; if the shading ticket finds the
  notes specify a conversion, it is recorded there.

## 5. Non-goals (honest)

No ray tracing, no path tracing, no GPU / shader
programs, no real-time or animated rendering, no
mesh types other than triangulated `.obj`, no output
format other than TGA, no indirect lighting, no toon
shading (both only as unverified later lessons — see
README), no texture formats beyond what the supplied
TGA class and the `.obj` textures require, and no
scene-file format beyond the `.obj` files themselves.
