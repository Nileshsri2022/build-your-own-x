// Device: back buffer, depth buffer, render loop, wireframe (Bresenham)
// and scanline rasterization. Written from SPEC/DESIGN; the tutorial's
// routine names ProcessScanLine / Interpolate / Clamp are kept for
// traceability, implementations are our own.
import { Vector3, Matrix, Clamp, Interpolate, ComputeNDotL } from "./math.js";

export class Device {
  constructor(width, height, background = [0, 0, 0]) {
    this.width = width; this.height = height;
    this.background = background;
    this.backBuffer = new Uint8ClampedArray(width * height * 3);
    this.depthBuffer = new Float64Array(width * height);
    this.clear();
  }
  clear() {
    for (let i = 0; i < this.width * this.height; i++) {
      this.backBuffer[i * 3] = this.background[0];
      this.backBuffer[i * 3 + 1] = this.background[1];
      this.backBuffer[i * 3 + 2] = this.background[2];
      this.depthBuffer[i] = Infinity; // pinned by depth tests: smaller = nearer
    }
  }
  pixel(x, y) {
    const i = (y * this.width + x) * 3;
    return [this.backBuffer[i], this.backBuffer[i + 1], this.backBuffer[i + 2]];
  }
  putPixel(x, y, color) {
    if (x < 0 || y < 0 || x >= this.width || y >= this.height) return; // point clipping
    const i = (y * this.width + x) * 3;
    this.backBuffer[i] = color[0]; this.backBuffer[i + 1] = color[1]; this.backBuffer[i + 2] = color[2];
  }
  // Bresenham, all octants, both endpoints inclusive.
  drawLine(x0, y0, x1, y1, color) {
    let dx = Math.abs(x1 - x0), dy = -Math.abs(y1 - y0);
    const sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    let err = dx + dy;
    for (;;) {
      this.putPixel(x0, y0, color);
      if (x0 === x1 && y0 === y1) break;
      const e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
    }
  }

  // Project every vertex of a mesh to screen space. Returns null for a
  // vertex behind the camera (w <= 0); such faces are skipped, never crash.
  project(camera, mesh) {
    const world = Matrix.translation(mesh.position).multiply(Matrix.rotationEuler(mesh.rotation));
    const view = Matrix.lookAtLH(camera.position, camera.target);
    const proj = Matrix.perspectiveFovLH(camera.fov, this.width / this.height, camera.near, camera.far);
    const wvp = world.multiply(view).multiply(proj);
    const rot = Matrix.rotationEuler(mesh.rotation);
    return mesh.vertices.map((v, i) => {
      const clip = wvp.transformPoint(v);
      if (clip.w <= 0) return null;
      const nx = clip.x / clip.w, ny = clip.y / clip.w, nz = clip.z / clip.w;
      return {
        x: (nx + 1) / 2 * this.width,
        y: (1 - ny) / 2 * this.height,
        depth: nz,
        world: world.transformDirection(v).add(mesh.position),
        normal: rot.transformDirection(mesh.normals[i]).normalize(),
        uv: mesh.uvs ? mesh.uvs[i] : null,
      };
    });
  }

  render(camera, meshes, options = {}) {
    const mode = options.mode ?? "gouraud"; // wireframe | flat | gouraud
    const culling = options.culling ?? false;
    const light = options.light ?? new Vector3(0, 5, 10);
    this.clear();
    for (const mesh of meshes) {
      const projected = this.project(camera, mesh);
      if (mode === "wireframe") {
        // Feature-edge wireframe, not one outline per triangle: a
        // triangulated square face shares a diagonal edge between its
        // two triangles, and per-triangle lines would draw all 6 such
        // diagonals across the cube. TEST_PLAN requires 12 visible
        // edges with face interiors left background, so an edge
        // shared by two coplanar faces (same geometric normal) is
        // skipped, and only true feature edges are drawn.
        const edgeFaces = new Map();
        for (const [ia, ib, ic] of mesh.faces) {
          const n = mesh.vertices[ib].sub(mesh.vertices[ia])
            .cross(mesh.vertices[ic].sub(mesh.vertices[ia])).normalize();
          for (const [u, v] of [[ia, ib], [ib, ic], [ic, ia]]) {
            const key = u < v ? `${u}-${v}` : `${v}-${u}`;
            if (!edgeFaces.has(key)) edgeFaces.set(key, { u, v, normals: [] });
            edgeFaces.get(key).normals.push(n);
          }
        }
        for (const { u, v, normals } of edgeFaces.values()) {
          if (normals.length === 2 && normals[0].dot(normals[1]) > 0.999) continue;
          const pa = projected[u], pb = projected[v];
          if (!pa || !pb) continue;
          this.drawLine(Math.round(pa.x), Math.round(pa.y), Math.round(pb.x), Math.round(pb.y), [255, 255, 255]);
        }
        continue;
      }
      for (const [ia, ib, ic] of mesh.faces) {
        const pa = projected[ia], pb = projected[ib], pc = projected[ic];
        if (!pa || !pb || !pc) continue; // behind camera: skip, no crash
        if (culling) {
          // World-space test (avoids the tutorial's reported bug, where
          // the normal transform ignored camera perspective): cull only
          // when the face normal points along camera -> face, i.e. away.
          const fn = mesh.vertices[ib].sub(mesh.vertices[ia])
            .cross(mesh.vertices[ic].sub(mesh.vertices[ia]));
          const worldNormal = Matrix.rotationEuler(mesh.rotation)
            .transformDirection(fn).normalize();
          if (worldNormal.dot(pa.world.sub(camera.position)) > 0) continue;
        }
        this.rasterize([pa, pb, pc], mesh, mode, light);
      }
    }
  }

  // Scanline fill. Vertices carry screen x/y, depth, world position,
  // normal and uv; every attribute is interpolated linearly across
  // scanlines and then across each scanline (screen-space, as taught;
  // NOT perspective-correct — noted as a simplification vs the
  // tutorial's unverified Part 6 interpolation detail).
  rasterize(tri, mesh, mode, light) {
    const sorted = [...tri].sort((a, b) => a.y - b.y);
    const [top, mid, bot] = sorted;
    if (bot.y - top.y < 1e-9) return;
    // Flat shading: one shade for the whole face, from the geometric
    // face normal and the face centroid (SPEC: average-of-vertex-normals
    // face normal; the geometric normal equals that for a planar face).
    // The normal uses the face's ORIGINAL winding (tri order), not the
    // y-sorted order — sorting can flip the winding and the sign.
    let flatShade = 1;
    if (mode === "flat") {
      const fn = tri[1].world.sub(tri[0].world)
        .cross(tri[2].world.sub(tri[0].world)).normalize();
      const centroid = tri[0].world.add(tri[1].world).add(tri[2].world).scale(1 / 3);
      flatShade = ComputeNDotL(fn, centroid, light);
    }
    const drawRow = (y) => {
      const yc = y + 0.5;
      if (yc < top.y || yc > bot.y) return;
      const edge = (a, b) => {
        const g = b.y === a.y ? 0 : Clamp((yc - a.y) / (b.y - a.y));
        return lerpVertex(a, b, g);
      };
      let left, right;
      if (yc < mid.y) { left = edge(top, bot); right = edge(top, mid); }
      else { left = edge(top, bot); right = edge(mid, bot); }
      if (left.x > right.x) [left, right] = [right, left];
      for (let x = Math.ceil(left.x - 0.5); x + 0.5 <= right.x; x++) {
        if (x < 0 || x >= this.width) continue;
        const g = right.x === left.x ? 0 : Clamp((x + 0.5 - left.x) / (right.x - left.x));
        const p = lerpVertex(left, right, g);
        const idx = y * this.width + x;
        if (!(p.depth < this.depthBuffer[idx])) continue; // nearer wins
        let color;
        if (mode === "flat") {
          color = mesh.color.map((c) => c * flatShade);
        } else {
          const shade = ComputeNDotL(p.normal, p.world, light);
          color = mesh.color.map((c) => c * shade);
        }
        if (mesh.texture && p.uv) {
          const texel = mesh.texture.sample(p.uv[0], p.uv[1]);
          color = color.map((c, i) => c * texel[i]); // shade x texel, pinned
        }
        const o = idx * 3;
        this.backBuffer[o] = Math.round(Clamp(color[0]) * 255);
        this.backBuffer[o + 1] = Math.round(Clamp(color[1]) * 255);
        this.backBuffer[o + 2] = Math.round(Clamp(color[2]) * 255);
        this.depthBuffer[idx] = p.depth;
      }
    };
    for (let y = Math.floor(top.y); y <= Math.ceil(bot.y); y++) drawRow(y);
  }

  // P3 ASCII PPM of the current back buffer. Harness-only output
  // (TEST_PLAN: frames captured by the test harness, not an engine
  // feature — the engine itself has no image-file export).
  toPPM() {
    let out = `P3\n${this.width} ${this.height}\n255\n`;
    for (let y = 0; y < this.height; y++) {
      const row = [];
      for (let x = 0; x < this.width; x++) row.push(this.pixel(x, y).join(" "));
      out += row.join(" ") + "\n";
    }
    return out;
  }
}

// ProcessScanLine's per-scanline step, generalised: interpolate every
// carried attribute between two vertices by gradient g.
function lerpVertex(a, b, g) {
  const n = a.normal, m = b.normal;
  return {
    x: Interpolate(a.x, b.x, g),
    y: Interpolate(a.y, b.y, g),
    depth: Interpolate(a.depth, b.depth, g),
    world: new Vector3(
      Interpolate(a.world.x, b.world.x, g),
      Interpolate(a.world.y, b.world.y, g),
      Interpolate(a.world.z, b.world.z, g),
    ),
    normal: new Vector3(
      Interpolate(n.x, m.x, g),
      Interpolate(n.y, m.y, g),
      Interpolate(n.z, m.z, g),
    ).normalize(),
    uv: a.uv && b.uv ? [Interpolate(a.uv[0], b.uv[0], g), Interpolate(a.uv[1], b.uv[1], g)] : null,
  };
}
