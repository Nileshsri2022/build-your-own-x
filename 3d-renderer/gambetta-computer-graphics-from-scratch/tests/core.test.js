// T1 shared-core tests (TEST_PLAN section 2).
const test = require('node:test');
const assert = require('node:assert/strict');
const { Canvas } = require('../src/canvas');
const { rgb, add, scale, multiply, clamp } = require('../src/color');
const { Vec3, Mat4 } = require('../src/math');

test('canvas: putPixel then getPixel returns the colour written', () => {
  const c = new Canvas(10, 10);
  c.putPixel(3, 4, rgb(10, 20, 30));
  assert.deepEqual(c.getPixel(3, 4), { r: 10, g: 20, b: 30 });
});

test('canvas: out-of-bounds writes are ignored, neighbours uncorrupted', () => {
  const c = new Canvas(4, 4);
  c.putPixel(0, 0, rgb(1, 2, 3));
  c.putPixel(-1, 0, rgb(255, 255, 255));
  c.putPixel(4, 0, rgb(255, 255, 255));
  c.putPixel(0, -1, rgb(255, 255, 255));
  c.putPixel(0, 4, rgb(255, 255, 255));
  assert.deepEqual(c.getPixel(0, 0), { r: 1, g: 2, b: 3 });
  assert.equal(c.getPixel(-1, 0), null);
});

test('canvas: colours are clamped at PutPixel, not inside arithmetic', () => {
  const c = new Canvas(2, 2);
  c.putPixel(0, 0, rgb(300, -5, 128));
  assert.deepEqual(c.getPixel(0, 0), { r: 255, g: 0, b: 128 });
});

test('color: add, scale, multiply-by-white, clamp', () => {
  assert.deepEqual(add(rgb(10, 20, 30), rgb(1, 2, 3)), rgb(11, 22, 33));
  assert.deepEqual(scale(rgb(10, 20, 30), 0), rgb(0, 0, 0));
  assert.deepEqual(multiply(rgb(10, 20, 30), rgb(255, 255, 255)), rgb(10, 20, 30));
  assert.deepEqual(clamp(rgb(300, -5, 128)), rgb(255, 0, 128));
});

test('math: vector ops, dot, length, normalisation', () => {
  const a = new Vec3(1, 2, 3);
  assert.deepEqual(a.add(new Vec3(4, 5, 6)), new Vec3(5, 7, 9));
  assert.deepEqual(a.sub(new Vec3(1, 1, 1)), new Vec3(0, 1, 2));
  assert.deepEqual(a.scale(2), new Vec3(2, 4, 6));
  assert.equal(new Vec3(1, 0, 0).dot(new Vec3(0, 1, 0)), 0);
  assert.equal(new Vec3(2, 0, 0).dot(new Vec3(3, 0, 0)), 6);
  assert.equal(new Vec3(3, 4, 0).length(), 5);
  assert.equal(new Vec3(3, 4, 0).normalize().length(), 1);
  // Right-handedness: x cross y = z. Flipping the convention breaks this.
  assert.deepEqual(new Vec3(1, 0, 0).cross(new Vec3(0, 1, 0)), new Vec3(0, 0, 1));
});

test('math: matrix identity, translation, product order, rotationY', () => {
  const p = new Vec3(1, 2, 3);
  assert.deepEqual(Mat4.identity().transformPoint(p), p);
  assert.deepEqual(Mat4.translation(5, -2, 1).transformPoint(p), new Vec3(6, 0, 4));
  // Product order: T * S applied to a point scales first, then translates.
  const ts = Mat4.translation(10, 0, 0).multiply(Mat4.scaling(2, 2, 2));
  assert.deepEqual(ts.transformPoint(new Vec3(1, 0, 0)), new Vec3(12, 0, 0));
  // rotationY(+90deg) sends +z to +x in this right-handed convention
  // (within floating-point noise: cos(pi/2) is ~6e-17, not exactly 0).
  const rotated = Mat4.rotationY(Math.PI / 2).transformPoint(new Vec3(0, 0, 1));
  assert.ok(Math.abs(rotated.x - 1) < 1e-9 && Math.abs(rotated.y) < 1e-9
    && Math.abs(rotated.z) < 1e-9, `rotationY result ${JSON.stringify(rotated)}`);
  // Vectors ignore translation.
  assert.deepEqual(
    Mat4.translation(9, 9, 9).transformVector(new Vec3(1, 2, 3)),
    new Vec3(1, 2, 3),
  );
});
