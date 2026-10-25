// Lines (Bresenham) and triangle rasterization.
// Depth buffer: one view-space depth per pixel, initialised to Infinity;
// a pixel is written only when the new depth is STRICTLY nearer. Strict
// comparison is what makes shared edges safe: the second triangle of a
// shared edge has equal depth there, so it cannot overwrite the first,
// and no pixel is left uncoloured or double-coloured.
// Degenerate (zero-area) triangles throw — they must fail loudly, not
// render a silently wrong image. Unknown shading modes throw likewise.
// Attribute interpolation is perspective-correct (via 1/z) for colours,
// normals and UVs, except when texture mode 'linear' is explicitly chosen
// for the contrast test.
const { Vec3 } = require('../math');
const { add, scale, multiply } = require('../color');
const { computeLight } = require('./shade');
const { Texture } = require('./texture');

function makeDepthBuffer(width, height) {
  return new Float64Array(width * height).fill(Infinity);
}

function drawLine(canvas, x0, y0, x1, y1, color) {
  let ax = Math.round(x0), ay = Math.round(y0);
  const bx = Math.round(x1), by = Math.round(y1);
  const dx = Math.abs(bx - ax), dy = -Math.abs(by - ay);
  const sx = ax < bx ? 1 : -1, sy = ay < by ? 1 : -1;
  let err = dx + dy;
  for (;;) {
    canvas.putPixel(ax, ay, color);
    if (ax === bx && ay === by) break;
    const e2 = 2 * err;
    if (e2 >= dy) { err += dy; ax += sx; }
    if (e2 <= dx) { err += dx; ay += sy; }
  }
}

// Screen-space vertices: [{x, y, z (view depth), color?, normal?, uv?}]
// plus a per-triangle context: { lights, specular, baseColor, faceNormal,
// texture, textureMode, shading }. For 'flat', vertex colours are ignored
// and flatColor (computed by the caller) is used for every pixel.
function rasterizeTriangle(canvas, depthBuffer, verts, ctx) {
  if (!['flat', 'gouraud', 'phong'].includes(ctx.shading)) {
    throw new Error(`unknown shading mode: ${ctx.shading}`);
  }
  let [v0, v1, v2] = verts;
  const edge = (a, b, p) => (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
  let area = edge(v0, v1, v2);
  if (area === 0) throw new Error('degenerate triangle (zero area)');
  if (area < 0) { [v1, v2] = [v2, v1]; area = -area; }
  const vs = [v0, v1, v2];
  const minX = Math.max(0, Math.floor(Math.min(v0.x, v1.x, v2.x)));
  const maxX = Math.min(canvas.width - 1, Math.ceil(Math.max(v0.x, v1.x, v2.x)));
  const minY = Math.max(0, Math.floor(Math.min(v0.y, v1.y, v2.y)));
  const maxY = Math.min(canvas.height - 1, Math.ceil(Math.max(v0.y, v1.y, v2.y)));
  for (let y = minY; y <= maxY; y++) {
    for (let x = minX; x <= maxX; x++) {
      const p = { x: x + 0.5, y: y + 0.5 };
      const w0 = edge(v1, v2, p) / area;
      const w1 = edge(v2, v0, p) / area;
      const w2 = edge(v0, v1, p) / area;
      if (w0 < 0 || w1 < 0 || w2 < 0) continue;
      const weights = [w0, w1, w2];
      // Perspective-correct depth: interpolate 1/z, then invert.
      let iz = 0;
      for (let i = 0; i < 3; i++) iz += weights[i] / vs[i].z;
      const depth = 1 / iz;
      const di = y * canvas.width + x;
      if (!(depth < depthBuffer[di])) continue;
      let color;
      if (ctx.shading === 'flat') {
        color = ctx.flatColor;
      } else if (ctx.shading === 'gouraud') {
        color = { r: 0, g: 0, b: 0 };
        for (let i = 0; i < 3; i++) {
          const w = (weights[i] / vs[i].z) / iz;
          color = add(color, scale(vs[i].color, w));
        }
      } else { // phong: interpolate normals perspective-correctly
        let n = new Vec3(0, 0, 0);
        for (let i = 0; i < 3; i++) {
          const w = (weights[i] / vs[i].z) / iz;
          n = n.add(vs[i].normal.scale(w));
        }
        n = n.normalize();
        // View-space point for the view vector: render.js supplies
        // viewPointAt (inverse projection); without it, fall back to a
        // point on the view axis, which still gives the correct sign.
        const viewPoint = ctx.viewPointAt
          ? ctx.viewPointAt(p.x, p.y, depth) : new Vec3(0, 0, depth);
        const viewDir = viewPoint.negate().normalize();
        color = computeLight(
          viewPoint, n, viewDir, ctx.lights, ctx.baseColor, ctx.specular,
        );
      }
      if (ctx.texture && vs[0].uv) {
        const uv = Texture.interpolateUv(
          weights, vs.map((v) => v.z), vs.map((v) => v.uv), ctx.textureMode || 'perspective',
        );
        color = multiply(color, ctx.texture.sample(uv.u, uv.v));
      }
      depthBuffer[di] = depth;
      canvas.putPixel(x, y, color);
    }
  }
}
module.exports = { makeDepthBuffer, drawLine, rasterizeTriangle };
