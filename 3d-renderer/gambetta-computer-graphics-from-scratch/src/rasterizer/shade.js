// Rasterizer shading — a separate implementation from the raytracer's
// chapter-3 code, as TICKETS.md requires. Lights use the same types and
// the same white-only, no-falloff rule; unknown light types throw.
// FLAGGED, UNVERIFIED AGAINST CHAPTER 13 (docs: "to verify before code"):
// which normal each mode uses is an ADOPTED convention, matching the
// glossary's own definitions, not verified chapter text:
//   flat    — one colour per triangle, from the geometric face normal
//   Gouraud — lighting at each vertex from that vertex's normal,
//             colours interpolated across the triangle
//   Phong   — vertex normals interpolated per pixel, lighting per pixel
// If chapter 13 is read in full and differs, change this file and its tests.
const { add, scale } = require('../color');

function computeLight(point, normal, viewDir, lights, color, specular) {
  let intensity = 0;
  let spec = 0;
  for (const light of lights) {
    if (light.type === 'ambient') {
      intensity += light.intensity;
      continue;
    }
    let lightDir;
    if (light.type === 'point') {
      lightDir = light.position.sub(point).normalize();
    } else if (light.type === 'directional') {
      lightDir = light.direction.negate().normalize();
    } else {
      throw new Error(`unknown light type: ${light.type}`);
    }
    const nDotL = normal.dot(lightDir);
    if (nDotL > 0) intensity += light.intensity * nDotL;
    if (specular >= 0) {
      const reflectDir = normal.scale(2 * nDotL).sub(lightDir);
      const rDotV = reflectDir.dot(viewDir);
      if (rDotV > 0) spec += light.intensity * Math.pow(rDotV, specular);
    }
  }
  // Specular is white light: scale the 0-1 term into 0-255 colour space.
  return add(scale(color, intensity), { r: spec * 255, g: spec * 255, b: spec * 255 });
}
module.exports = { computeLight };
