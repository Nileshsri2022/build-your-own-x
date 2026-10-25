// Camera, Mesh, Texture and the Blender-JSON loader. Written from SPEC;
// tutorial object names (Camera, Mesh, Texture) are used for traceability.
import { Vector3 } from "./math.js";

export class Camera {
  constructor(position, target, fov = 0.78, near = 0.01, far = 1.0) {
    this.position = position;
    this.target = target;
    this.fov = fov; this.near = near; this.far = far;
  }
}

export class Mesh {
  // vertices: Vector3[]; faces: [i, j, k][]; normals: per-vertex
  // Vector3[] (optional, computed if absent); uvs: [u, v][] (optional).
  // color: [r, g, b] in [0, 1]. position/rotation: world transform.
  constructor(name, vertices, faces, options = {}) {
    this.name = name;
    this.vertices = vertices;
    this.faces = faces;
    this.normals = options.normals ?? null;
    this.uvs = options.uvs ?? null;
    this.position = options.position ?? new Vector3(0, 0, 0);
    this.rotation = options.rotation ?? new Vector3(0, 0, 0);
    this.color = options.color ?? [1, 1, 1];
    this.texture = options.texture ?? null;
    if (!this.normals) this.normals = computeVertexNormals(vertices, faces);
  }
}

function faceNormalOf(p0, p1, p2) {
  return p1.sub(p0).cross(p2.sub(p0)).normalize();
}

export function computeVertexNormals(vertices, faces) {
  const acc = vertices.map(() => new Vector3(0, 0, 0));
  for (const [i, j, k] of faces) {
    const n = faceNormalOf(vertices[i], vertices[j], vertices[k]);
    acc[i] = acc[i].add(n); acc[j] = acc[j].add(n); acc[k] = acc[k].add(n);
  }
  return acc.map((v) => v.normalize());
}

// Scene A (Part 1): hand-built cube, 8 vertices, 12 triangular faces.
// Winding chosen so cross(v1-v0, v2-v0) points outward per face.
export function cubeMesh() {
  const v = [
    [-1, -1, -1], [1, -1, -1], [1, 1, -1], [-1, 1, -1],
    [-1, -1, 1], [1, -1, 1], [1, 1, 1], [-1, 1, 1],
  ].map(([x, y, z]) => new Vector3(x, y, z));
  const faces = [
    [0, 2, 1], [0, 3, 2],       // back  (z = -1)
    [4, 5, 6], [4, 6, 7],       // front (z = +1)
    [0, 4, 7], [0, 7, 3],       // left  (x = -1)
    [1, 2, 6], [1, 6, 5],       // right (x = +1)
    [0, 1, 5], [0, 5, 4],       // bottom (y = -1)
    [3, 6, 2], [3, 7, 6],       // top   (y = +1)
  ];
  return new Mesh("cube", v, faces);
}

export class Texture {
  // data: Uint8ClampedArray of RGBA bytes, row-major from the top.
  constructor(width, height, data) {
    if (data.length !== width * height * 4) {
      throw new Error(`Texture data length ${data.length} != ${width * height * 4}`);
    }
    this.width = width; this.height = height; this.data = data;
  }
  static checker(squaresPerSide = 2, size = 64) {
    const data = new Uint8ClampedArray(size * size * 4);
    const step = size / squaresPerSide;
    for (let y = 0; y < size; y++) {
      for (let x = 0; x < size; x++) {
        const on = (Math.floor(x / step) + Math.floor(y / step)) % 2 === 0;
        const i = (y * size + x) * 4;
        data[i] = data[i + 1] = data[i + 2] = on ? 255 : 0;
        data[i + 3] = 255;
      }
    }
    return new Texture(size, size, data);
  }
  // Modulo wrap (taught behaviour): u = 1.25 -> 0.25, u = -0.25 -> 0.75.
  sample(u, v) {
    const wu = ((u % 1) + 1) % 1, wv = ((v % 1) + 1) % 1;
    const x = Math.min(this.width - 1, Math.floor(wu * this.width));
    const y = Math.min(this.height - 1, Math.floor(wv * this.height));
    const i = (y * this.width + x) * 4;
    return [this.data[i] / 255, this.data[i + 1] / 255, this.data[i + 2] / 255];
  }
}

// Blender-JSON loader (Part 3). Accepts the exporter's shape flexibly:
// {meshes:[{name, vertices, faces, normals?, uvs?}]} or a single mesh
// object. Vertices may be a flat number array or [x,y,z] triples;
// faces may be index triples or a flat array grouped by 3; quads are
// triangulated (a, b, c), (a, c, d). Anything missing, malformed, or
// referencing an out-of-range vertex throws — never a half mesh.
export function loadMeshFromJson(obj) {
  if (!obj || typeof obj !== "object") throw new Error("Mesh JSON must be an object");
  const raw = Array.isArray(obj.meshes) ? obj.meshes[0] : obj;
  if (!raw || !Array.isArray(raw.vertices) || !Array.isArray(raw.faces)) {
    throw new Error("Mesh JSON needs vertices and faces");
  }
  let vertices;
  if (raw.vertices.length > 0 && typeof raw.vertices[0] === "number") {
    if (raw.vertices.length % 3 !== 0) throw new Error("Flat vertices length is not a multiple of 3");
    vertices = [];
    for (let i = 0; i < raw.vertices.length; i += 3) {
      vertices.push(new Vector3(raw.vertices[i], raw.vertices[i + 1], raw.vertices[i + 2]));
    }
  } else {
    vertices = raw.vertices.map((p) => {
      if (!Array.isArray(p) || p.length < 3) throw new Error("Vertex is not an [x, y, z] triple");
      return new Vector3(p[0], p[1], p[2]);
    });
  }
  if (vertices.length === 0) throw new Error("Mesh has no vertices");
  let faces = [];
  const flat = raw.faces.length > 0 && typeof raw.faces[0] === "number";
  const groups = flat ? chunk(raw.faces, 3) : raw.faces;
  for (const g of groups) {
    if (!Array.isArray(g) || (g.length !== 3 && g.length !== 4)) {
      throw new Error("Face is not a triple or quad of indices");
    }
    for (const idx of g) {
      if (!Number.isInteger(idx) || idx < 0 || idx >= vertices.length) {
        throw new Error(`Face index ${idx} out of range (0..${vertices.length - 1})`);
      }
    }
    faces.push([g[0], g[1], g[2]]);
    if (g.length === 4) faces.push([g[0], g[2], g[3]]);
  }
  if (faces.length === 0) throw new Error("Mesh has no faces");
  let normals = null;
  if (Array.isArray(raw.normals) && raw.normals.length > 0) {
    normals = typeof raw.normals[0] === "number"
      ? chunk(raw.normals, 3).map(([x, y, z]) => new Vector3(x, y, z))
      : raw.normals.map(([x, y, z]) => new Vector3(x, y, z));
    if (normals.length !== vertices.length) throw new Error("Normals count != vertices count");
  }
  let uvs = null;
  if (Array.isArray(raw.uvs) && raw.uvs.length > 0) {
    uvs = typeof raw.uvs[0] === "number"
      ? chunk(raw.uvs, 2)
      : raw.uvs;
    if (uvs.length !== vertices.length) throw new Error("UV count != vertices count");
  }
  return new Mesh(raw.name ?? "imported", vertices, faces, { normals, uvs });
}

function chunk(arr, n) {
  const out = [];
  for (let i = 0; i < arr.length; i += n) out.push(arr.slice(i, i + n));
  return out;
}
