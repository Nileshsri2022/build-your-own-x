// Clipping against the near plane (view-space z >= near) by
// Sutherland-Hodgman. Vertex attributes (color, normal, uv) are linearly
// interpolated at the crossing point. A polygon with < 3 vertices after
// clipping is empty. Instance-level clipping (whole instance behind the
// near plane) is done in render.js before triangles are visited.
function lerpAttr(a, b, t, lerp) {
  if (a === undefined) return undefined;
  return lerp(a, b, t);
}
function clipPolygonAgainstNear(polygon, near) {
  if (polygon.length === 0) return [];
  const out = [];
  const inside = (v) => v.pos.z >= near;
  for (let i = 0; i < polygon.length; i++) {
    const cur = polygon[i];
    const prev = polygon[(i + polygon.length - 1) % polygon.length];
    const curIn = inside(cur);
    const prevIn = inside(prev);
    if (curIn !== prevIn) {
      const t = (near - prev.pos.z) / (cur.pos.z - prev.pos.z);
      const lerpVec = (a, b) => a.add(b.sub(a).scale(t));
      const lerpCol = (a, b) => ({
        r: a.r + (b.r - a.r) * t, g: a.g + (b.g - a.g) * t, b: a.b + (b.b - a.b) * t,
      });
      const lerpUv = (a, b) => ({ u: a.u + (b.u - a.u) * t, v: a.v + (b.v - a.v) * t });
      out.push({
        pos: lerpVec(prev.pos, cur.pos),
        color: lerpAttr(prev.color, cur.color, t, lerpCol),
        normal: lerpAttr(prev.normal, cur.normal, t, lerpVec),
        uv: lerpAttr(prev.uv, cur.uv, t, lerpUv),
      });
    }
    if (curIn) out.push(cur);
  }
  return out;
}
module.exports = { clipPolygonAgainstNear };
