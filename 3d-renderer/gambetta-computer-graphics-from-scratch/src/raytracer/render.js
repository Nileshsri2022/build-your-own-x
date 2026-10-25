// Raytracer render loop: one ray per pixel through a viewport plane.
// Camera at the origin looking along +z; viewport is vw x vh at distance d.
// Canvas coordinates are centred: pixel (0,0) is the top-left corner, so
// canvas x maps to viewport x left-to-right and canvas y maps to viewport
// y top-to-bottom (negated, since viewport y is up).
const { Vec3 } = require('../math');
const { Ray } = require('./ray');
const { traceRay } = require('./trace');

function renderRaytracer(canvas, scene, {
  viewportWidth = 1, viewportHeight = 1, viewportDistance = 1, recursionDepth = 3,
} = {}) {
  const camera = new Vec3(0, 0, 0);
  for (let cy = 0; cy < canvas.height; cy++) {
    for (let cx = 0; cx < canvas.width; cx++) {
      const vx = (cx - canvas.width / 2 + 0.5) * (viewportWidth / canvas.width);
      const vy = -(cy - canvas.height / 2 + 0.5) * (viewportHeight / canvas.height);
      const dir = new Vec3(vx, vy, viewportDistance);
      canvas.putPixel(cx, cy, traceRay(scene, new Ray(camera, dir), recursionDepth));
    }
  }
  return canvas;
}
module.exports = { renderRaytracer };
