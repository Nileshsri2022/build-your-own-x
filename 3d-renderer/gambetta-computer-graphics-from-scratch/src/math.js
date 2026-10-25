// Vectors and 4x4 matrices. ONE convention, fixed here, used by both
// renderers: right-handed coordinates (x right, y up, z toward the viewer
// in view space for the rasterizer, and toward the scene for the
// raytracer — both use the same vector math; only the camera placement
// differs), row-major Mat4 storage, column vectors: a transformed point
// is M * p, and multiply(a, b) means "apply b first, then a".
class Vec3 {
  constructor(x, y, z) { this.x = x; this.y = y; this.z = z; }
  add(v) { return new Vec3(this.x + v.x, this.y + v.y, this.z + v.z); }
  sub(v) { return new Vec3(this.x - v.x, this.y - v.y, this.z - v.z); }
  scale(k) { return new Vec3(this.x * k, this.y * k, this.z * k); }
  dot(v) { return this.x * v.x + this.y * v.y + this.z * v.z; }
  cross(v) {
    return new Vec3(
      this.y * v.z - this.z * v.y,
      this.z * v.x - this.x * v.z,
      this.x * v.y - this.y * v.x,
    );
  }
  length() { return Math.sqrt(this.dot(this)); }
  normalize() {
    const l = this.length();
    return l === 0 ? new Vec3(0, 0, 0) : this.scale(1 / l);
  }
  negate() { return this.scale(-1); }
}
class Mat4 {
  // elements: row-major, 16 numbers.
  constructor(elements) { this.e = elements.slice(); }
  static identity() {
    return new Mat4([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  static translation(x, y, z) {
    return new Mat4([1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1]);
  }
  static scaling(sx, sy, sz) {
    return new Mat4([sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1]);
  }
  static rotationX(a) {
    const c = Math.cos(a), s = Math.sin(a);
    return new Mat4([1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1]);
  }
  static rotationY(a) {
    const c = Math.cos(a), s = Math.sin(a);
    return new Mat4([c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1]);
  }
  static rotationZ(a) {
    const c = Math.cos(a), s = Math.sin(a);
    return new Mat4([c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  // Returns a * b (apply b first, then a).
  multiply(b) {
    const a = this.e, c = b.e, out = new Array(16).fill(0);
    for (let r = 0; r < 4; r++) {
      for (let col = 0; col < 4; col++) {
        for (let k = 0; k < 4; k++) out[r * 4 + col] += a[r * 4 + k] * c[k * 4 + col];
      }
    }
    return new Mat4(out);
  }
  transformPoint(p) {
    const e = this.e;
    return new Vec3(
      e[0] * p.x + e[1] * p.y + e[2] * p.z + e[3],
      e[4] * p.x + e[5] * p.y + e[6] * p.z + e[7],
      e[8] * p.x + e[9] * p.y + e[10] * p.z + e[11],
    );
  }
  // Vectors ignore translation.
  transformVector(v) {
    const e = this.e;
    return new Vec3(
      e[0] * v.x + e[1] * v.y + e[2] * v.z,
      e[4] * v.x + e[5] * v.y + e[6] * v.z,
      e[8] * v.x + e[9] * v.y + e[10] * v.z,
    );
  }
  // Rotation-only part, transposed (inverse of a pure rotation).
  transposeRotation() {
    const e = this.e;
    return new Mat4([
      e[0], e[4], e[8], 0,
      e[1], e[5], e[9], 0,
      e[2], e[6], e[10], 0,
      0, 0, 0, 1,
    ]);
  }
}
module.exports = { Vec3, Mat4 };
