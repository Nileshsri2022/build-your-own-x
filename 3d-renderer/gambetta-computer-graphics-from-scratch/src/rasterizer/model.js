// Model: vertex list + triangle list. A triangle references three vertices
// and carries one face colour and one UV per corner. Vertex normals are
// derived (average of adjacent face normals) for Gouraud/Phong shading;
// flat shading uses the geometric face normal instead (see shade.js).
const { Vec3 } = require('../math');

class Model {
  constructor(vertices, triangles) {
    this.vertices = vertices; // Vec3[]
    // triangles: [{ indices: [i,j,k], color: {r,g,b}, uvs: [{u,v} x3] }]
    this.triangles = triangles;
    this.vertexNormals = this.computeVertexNormals();
  }
  faceNormal(tri) {
    const [a, b, c] = tri.indices.map((i) => this.vertices[i]);
    return b.sub(a).cross(c.sub(a)).normalize();
  }
  computeVertexNormals() {
    const normals = this.vertices.map(() => new Vec3(0, 0, 0));
    for (const tri of this.triangles) {
      const n = this.faceNormal(tri);
      for (const i of tri.indices) normals[i] = normals[i].add(n);
    }
    return normals.map((n) => (n.length() === 0 ? new Vec3(0, 0, 1) : n.normalize()));
  }
}

// Default cube, side 2, centred on the origin. Each face is listed as four
// corners in cyclic order seen from outside that face, so both triangles
// of the face have an outward-pointing geometric normal (checked by test).
function makeCube() {
  const V = (x, y, z) => new Vec3(x, y, z);
  const vertices = [
    V(-1, -1, -1), V(1, -1, -1), V(1, 1, -1), V(-1, 1, -1), // z = -1: 0..3
    V(-1, -1, 1), V(1, -1, 1), V(1, 1, 1), V(-1, 1, 1), // z = +1: 4..7
  ];
  const faces = [
    { corners: [0, 3, 2, 1], color: { r: 255, g: 0, b: 0 } }, // -z, red
    { corners: [4, 5, 6, 7], color: { r: 0, g: 255, b: 0 } }, // +z, green
    { corners: [0, 4, 7, 3], color: { r: 0, g: 0, b: 255 } }, // -x, blue
    { corners: [5, 1, 2, 6], color: { r: 255, g: 255, b: 0 } }, // +x, yellow
    { corners: [0, 1, 5, 4], color: { r: 255, g: 0, b: 255 } }, // -y, magenta
    { corners: [7, 6, 2, 3], color: { r: 0, g: 255, b: 255 } }, // +y, cyan
  ];
  const uvs = [{ u: 0, v: 0 }, { u: 1, v: 0 }, { u: 1, v: 1 }, { u: 0, v: 1 }];
  const triangles = [];
  for (const face of faces) {
    const [a, b, c, d] = face.corners;
    triangles.push(
      { indices: [a, b, c], color: face.color, uvs: [uvs[0], uvs[1], uvs[2]] },
      { indices: [a, c, d], color: face.color, uvs: [uvs[0], uvs[2], uvs[3]] },
    );
  }
  return new Model(vertices, triangles);
}

module.exports = { Model, makeCube };
