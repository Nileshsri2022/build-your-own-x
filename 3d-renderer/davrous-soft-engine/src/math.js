// Minimal vector/matrix maths for the Davrous soft engine, written from
// SPEC/DESIGN (not copied from the tutorial). Row-vector convention, as
// in the tutorial's matrix libraries: a point is transformed as v * M,
// so world * view * projection applies world first, then view, then
// projection. Left-handed: the camera's forward axis is
// normalize(target - position) and view-space z is distance along it.

export class Vector3 {
  constructor(x = 0, y = 0, z = 0) { this.x = x; this.y = y; this.z = z; }
  add(v) { return new Vector3(this.x + v.x, this.y + v.y, this.z + v.z); }
  sub(v) { return new Vector3(this.x - v.x, this.y - v.y, this.z - v.z); }
  scale(s) { return new Vector3(this.x * s, this.y * s, this.z * s); }
  dot(v) { return this.x * v.x + this.y * v.y + this.z * v.z; }
  cross(v) {
    return new Vector3(
      this.y * v.z - this.z * v.y,
      this.z * v.x - this.x * v.z,
      this.x * v.y - this.y * v.x,
    );
  }
  length() { return Math.hypot(this.x, this.y, this.z); }
  normalize() {
    const l = this.length();
    return l === 0 ? new Vector3(0, 0, 0) : this.scale(1 / l);
  }
}

// 4x4 matrix, row-major: m[row * 4 + col]. Row vector v=(x,y,z,1) gives
// out_j = x*m[j] + y*m[4+j] + z*m[8+j] + m[12+j].
export class Matrix {
  constructor(values) { this.m = values ? Float64Array.from(values) : new Float64Array(16); }
  static identity() {
    const m = new Matrix();
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1;
    return m;
  }
  static translation(p) {
    const m = Matrix.identity();
    m.m[12] = p.x; m.m[13] = p.y; m.m[14] = p.z;
    return m;
  }
  static rotationX(a) {
    const m = Matrix.identity(), c = Math.cos(a), s = Math.sin(a);
    m.m[5] = c; m.m[6] = s; m.m[9] = -s; m.m[10] = c;
    return m;
  }
  static rotationY(a) {
    const m = Matrix.identity(), c = Math.cos(a), s = Math.sin(a);
    m.m[0] = c; m.m[2] = -s; m.m[8] = s; m.m[10] = c;
    return m;
  }
  static rotationZ(a) {
    const m = Matrix.identity(), c = Math.cos(a), s = Math.sin(a);
    m.m[0] = c; m.m[1] = s; m.m[4] = -s; m.m[5] = c;
    return m;
  }
  // Euler order: Y, then X, then Z (row-vector order: v * Ry * Rx * Rz).
  static rotationEuler(e) {
    return Matrix.rotationY(e.y)
      .multiply(Matrix.rotationX(e.x))
      .multiply(Matrix.rotationZ(e.z));
  }
  static lookAtLH(position, target, up = new Vector3(0, 1, 0)) {
    const zAxis = target.sub(position).normalize();
    const xAxis = zAxis.cross(up).normalize();
    const yAxis = xAxis.cross(zAxis);
    const m = new Matrix();
    m.m[0] = xAxis.x; m.m[1] = yAxis.x; m.m[2] = zAxis.x; m.m[3] = 0;
    m.m[4] = xAxis.y; m.m[5] = yAxis.y; m.m[6] = zAxis.y; m.m[7] = 0;
    m.m[8] = xAxis.z; m.m[9] = yAxis.z; m.m[10] = zAxis.z; m.m[11] = 0;
    m.m[12] = -xAxis.dot(position);
    m.m[13] = -yAxis.dot(position);
    m.m[14] = -zAxis.dot(position);
    m.m[15] = 1;
    return m;
  }
  // DirectX-style left-handed perspective: out_w = view z, so a point
  // behind the camera (view z <= 0) is detected by w <= 0 and skipped.
  // Near/far as taught: fov 0.78, near 0.01, far 1.0. No far-plane
  // clipping is applied (SPEC records screen-space point clipping
  // only); projected z is used as the depth value, smaller = nearer.
  static perspectiveFovLH(fov, aspect, near, far) {
    const t = 1 / Math.tan(fov / 2);
    const m = new Matrix();
    m.m[0] = t / aspect; m.m[5] = t;
    m.m[10] = far / (far - near); m.m[11] = 1;
    m.m[14] = -near * far / (far - near);
    return m;
  }
  multiply(other) {
    const a = this.m, b = other.m, out = new Matrix();
    for (let row = 0; row < 4; row++) {
      for (let col = 0; col < 4; col++) {
        out.m[row * 4 + col] =
          a[row * 4] * b[col] + a[row * 4 + 1] * b[4 + col] +
          a[row * 4 + 2] * b[8 + col] + a[row * 4 + 3] * b[12 + col];
      }
    }
    return out;
  }
  // Returns {x, y, z, w} in clip space (divide by w for NDC).
  transformPoint(v) {
    const m = this.m;
    return {
      x: v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12],
      y: v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13],
      z: v.x * m[2] + v.y * m[6] + v.z * m[10] + m[14],
      w: v.x * m[3] + v.y * m[7] + v.z * m[11] + m[15],
    };
  }
  // Rotation-only transform (for normals: no translation, no projection).
  transformDirection(v) {
    const m = this.m;
    return new Vector3(
      v.x * m[0] + v.y * m[4] + v.z * m[8],
      v.x * m[1] + v.y * m[5] + v.z * m[9],
      v.x * m[2] + v.y * m[6] + v.z * m[10],
    );
  }
}

export function Clamp(value, min = 0, max = 1) {
  return Math.min(max, Math.max(min, value));
}

export function Interpolate(min, max, gradient) {
  return min + (max - min) * gradient;
}

// Tutorial's ComputeNDotL: max(0, cos(angle)) between the surface
// normal and the direction from the surface point to the light.
// Pinned convention (TEST_PLAN T5): light is a point, no falloff;
// the direction is surface -> light, computed per pixel/face.
export function ComputeNDotL(normal, worldPoint, lightPosition) {
  const toLight = lightPosition.sub(worldPoint).normalize();
  return Math.max(0, normal.normalize().dot(toLight));
}
