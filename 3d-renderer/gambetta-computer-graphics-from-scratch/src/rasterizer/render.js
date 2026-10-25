// Rasterizer render loop: per instance, transform model vertices by the
// instance transform then the camera transform; instance-level clip
// (whole instance behind the near plane); per triangle, face-normal
// culling (toggleable), near-plane clipping, projection, rasterization.
// FLAGGED, UNVERIFIED AGAINST CHAPTER 12 (docs: "to verify before code"):
// the "culling off" behaviour below is the ADOPTED reading of the demo
// description — with culling off, back faces are drawn (depth buffer
// still applies, so a back face behind a front face does not show
// through it; it shows only where no front face covers it). If the
// chapter-12 demo's "off" behaviour differs, this and its test must be
// rewritten to match it.
const { Vec3 } = require('../math');
const { computeLight } = require('./shade');
const { clipPolygonAgainstNear } = require('./clip');
const { makeDepthBuffer, rasterizeTriangle } = require('./raster');

function renderRasterizer(canvas, instances, camera, lights, {
  culling = true, shading = null, textureMode = 'perspective',
} = {}) {
  const depthBuffer = makeDepthBuffer(canvas.width, canvas.height);
  const viewMatrix = camera.getViewMatrix();
  // Lights are given in world space; geometry is rendered in view space,
  // so point positions and directional directions are view-transformed
  // here (directions ignore translation, as transformVector does).
  const viewLights = lights.map((l) => {
    if (l.type === 'point') {
      return { ...l, position: viewMatrix.transformPoint(l.position) };
    }
    if (l.type === 'directional') {
      return { ...l, direction: viewMatrix.transformVector(l.direction) };
    }
    return l;
  });
  lights = viewLights;
  for (const instance of instances) {
    const modelMatrix = instance.getTransform();
    const worldToView = viewMatrix.multiply(modelMatrix);
    const rotMatrix = viewMatrix.multiply(instance.getRotationMatrix());
    // Transform all vertices once.
    const viewVerts = instance.model.vertices.map((v) => worldToView.transformPoint(v));
    // Instance-level clipping: every vertex behind the near plane.
    if (viewVerts.every((v) => v.z < camera.near)) continue;
    const mode = shading || instance.shading || 'flat';
    for (const tri of instance.model.triangles) {
      const baseColor = instance.color || tri.color;
      const viewTri = tri.indices.map((vi, k) => ({
        pos: viewVerts[vi],
        normal: rotMatrix.transformVector(instance.model.vertexNormals[vi]).normalize(),
        uv: tri.uvs ? tri.uvs[k] : undefined,
      }));
      // Back-face culling in view space: a face is visible when its
      // normal points toward the camera (dot with -centroid > 0).
      const centroid = viewTri.reduce((a, v) => a.add(v.pos), new Vec3(0, 0, 0)).scale(1 / 3);
      const faceNormal = viewTri[1].pos.sub(viewTri[0].pos)
        .cross(viewTri[2].pos.sub(viewTri[0].pos)).normalize();
      if (culling && faceNormal.dot(centroid.negate()) <= 0) continue;
      // Triangle-level clipping against the near plane, then fan.
      const clipped = clipPolygonAgainstNear(viewTri, camera.near);
      if (clipped.length < 3) continue;
      // Per-vertex colours for Gouraud, computed once per (clipped) vertex.
      const withColors = clipped.map((v) => ({
        ...v,
        color: computeLight(
          v.pos, v.normal, v.pos.negate().normalize(), lights, baseColor, instance.specular,
        ),
      }));
      for (let i = 1; i + 1 < withColors.length; i++) {
        const poly = [withColors[0], withColors[i], withColors[i + 1]];
        const screen = poly.map((v) => {
          const s = camera.project(v.pos, canvas.width, canvas.height);
          return { x: s.x, y: s.y, z: v.pos.z, color: v.color, normal: v.normal, uv: v.uv };
        });
        const viewDirAtCentroid = centroid.negate().normalize();
        const flatColor = computeLight(
          centroid, faceNormal, viewDirAtCentroid, lights, baseColor, instance.specular,
        );
        rasterizeTriangle(canvas, depthBuffer, screen, {
          shading: mode,
          lights,
          baseColor,
          specular: instance.specular,
          flatColor,
          texture: instance.texture || null,
          textureMode,
          viewPointAt: (sx, sy, depth) => new Vec3(
            (sx - canvas.width / 2) * (camera.viewportWidth / canvas.width)
              * depth / camera.viewportDistance,
            -(sy - canvas.height / 2) * (camera.viewportHeight / canvas.height)
              * depth / camera.viewportDistance,
            depth,
          ),
        });
      }
    }
  }
  return canvas;
}
module.exports = { renderRasterizer };
