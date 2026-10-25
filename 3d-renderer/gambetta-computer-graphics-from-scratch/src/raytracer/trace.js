// TraceRay: nearest hit, shading, shadow test, reflection recursion.
// FLAGGED, UNVERIFIED AGAINST CHAPTER 4 (docs: "to verify before code"):
// the shadow-ray t range and self-shadowing epsilon below are ADOPTED
// values, not the book's verified text. Shadow rays start at t = 0.001
// from the hit point (so the hit surface itself is not an occluder) and
// run to the light for point lights, unbounded for directional lights.
// If chapter 4 is ever read in full and differs, change these two
// constants and the tests that pin them — do not silently keep them.
const SHADOW_T_MIN = 0.001;

const { Ray } = require('./ray');
const { add, scale } = require('../color');

function closestIntersection(scene, ray, tMin, tMax) {
  let closest = null;
  for (const sphere of scene.spheres) {
    const hit = sphere.intersect(ray, tMin, tMax);
    if (hit && (!closest || hit.t < closest.t)) closest = hit;
  }
  return closest;
}

function computeLighting(scene, hit, viewDir) {
  const { point, normal, sphere } = hit;
  let intensity = 0;
  let specular = 0;
  for (const light of scene.lights) {
    if (light.type === 'ambient') {
      intensity += light.intensity;
      continue;
    }
    let lightDir;
    let tMax;
    if (light.type === 'point') {
      const toLight = light.position.sub(point);
      tMax = toLight.length();
      lightDir = toLight.normalize();
    } else if (light.type === 'directional') {
      lightDir = light.direction.negate().normalize();
      tMax = Infinity;
    } else {
      throw new Error(`unknown light type: ${light.type}`);
    }
    // Shadow test: anything between the hit point and the light blocks it.
    const shadowRay = new Ray(point, lightDir);
    if (closestIntersection(scene, shadowRay, SHADOW_T_MIN, tMax)) continue;
    const nDotL = normal.dot(lightDir);
    if (nDotL > 0) intensity += light.intensity * nDotL;
    if (sphere.specular >= 0) {
      const reflectDir = normal.scale(2 * nDotL).sub(lightDir);
      const rDotV = reflectDir.dot(viewDir);
      if (rDotV > 0) specular += light.intensity * Math.pow(rDotV, sphere.specular);
    }
  }
  // Specular is white light: scale the 0-1 term into 0-255 colour space.
  return { diffuse: scale(sphere.color, intensity), specular: specular * 255 };
}

function traceRay(scene, ray, depth) {
  const hit = closestIntersection(scene, ray, 0.001, Infinity);
  if (!hit) return { ...scene.background };
  const viewDir = ray.direction.negate();
  const { diffuse, specular } = computeLighting(scene, hit, viewDir);
  let local = add(diffuse, { r: specular, g: specular, b: specular });
  const r = hit.sphere.reflective;
  if (r <= 0 || depth <= 0) return local;
  const reflectDir = ray.direction.sub(hit.normal.scale(2 * ray.direction.dot(hit.normal)));
  const reflected = traceRay(scene, new Ray(hit.point, reflectDir), depth - 1);
  return add(scale(local, 1 - r), scale(reflected, r));
}

module.exports = { traceRay, closestIntersection, computeLighting, SHADOW_T_MIN };
