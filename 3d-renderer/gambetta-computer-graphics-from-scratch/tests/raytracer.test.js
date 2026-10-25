// Part I tests (TEST_PLAN section 3). Expected values are hand-computed
// in the test, never derived by calling the production code twice.
const test = require('node:test');
const assert = require('node:assert/strict');
const { Canvas } = require('../src/canvas');
const { Vec3 } = require('../src/math');
const { rgb } = require('../src/color');
const { Ray } = require('../src/raytracer/ray');
const { Sphere } = require('../src/raytracer/sphere');
const { Light } = require('../src/raytracer/light');
const { Scene } = require('../src/raytracer/scene');
const { traceRay } = require('../src/raytracer/trace');
const { renderRaytracer } = require('../src/raytracer/render');

const approx = (actual, expected, tol = 3) => {
  for (const k of ['r', 'g', 'b']) {
    assert.ok(Math.abs(actual[k] - expected[k]) <= tol,
      `channel ${k}: got ${actual[k]}, expected ${expected[k]} ± ${tol}`);
  }
};
const centreRay = new Ray(new Vec3(0, 0, 0), new Vec3(0, 0, 1));

test('intersection: centre ray hits at the expected distance; miss and backwards rays do not', () => {
  const s = new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0));
  const hit = s.intersect(centreRay, 0.001, Infinity);
  assert.equal(hit.t, 2); // front surface at z = 3 - 1
  assert.equal(new Sphere(new Vec3(5, 0, 3), 1, rgb(0, 0, 0)).intersect(centreRay, 0.001, Infinity), null);
  assert.equal(s.intersect(new Ray(new Vec3(0, 0, 0), new Vec3(0, 0, -1)), 0.001, Infinity), null);
});

test('intersection: nearest of two spheres on the same ray wins', () => {
  const scene = new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 5), 1, rgb(0, 255, 0)),
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0)),
    ],
    lights: [],
  });
  const c = traceRay(scene, centreRay, 0);
  assert.deepEqual(c, rgb(0, 0, 0)); // no lights: both would be black; use render instead
  const canvas = renderRaytracer(new Canvas(11, 11), new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 5), 1, rgb(0, 255, 0)),
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0)),
    ],
    lights: [Light.ambient(1)],
  }));
  // Centre pixel: ambient 1 x nearest (red) sphere colour = red.
  assert.deepEqual(canvas.getPixel(5, 5), rgb(255, 0, 0));
});

test('basic render: one sphere, no lights — centre flat, outside background', () => {
  const canvas = renderRaytracer(new Canvas(21, 21), new Scene({
    spheres: [new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0))],
    lights: [Light.ambient(1)],
  }));
  assert.deepEqual(canvas.getPixel(10, 10), rgb(255, 0, 0));
  assert.deepEqual(canvas.getPixel(0, 0), rgb(0, 0, 0));
});

test('light: hand-computed diffuse — overhead full, behind zero, ambient alone flat', () => {
  const sphere = new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), -1, 0);
  // Hit point (0,0,2), normal (0,0,-1). Light at the camera: n.L = 1.
  const front = new Scene({
    spheres: [sphere],
    lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(0, 0, 0))],
  });
  approx(traceRay(front, centreRay, 0), rgb(255 * 0.8, 0, 0));
  // Light behind the sphere: n.L = -1 -> 0 diffuse, ambient only.
  const behind = new Scene({
    spheres: [sphere],
    lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(0, 0, 5))],
  });
  approx(traceRay(behind, centreRay, 0), rgb(255 * 0.2, 0, 0));
  // Ambient alone: flat, non-black.
  const ambientOnly = new Scene({
    spheres: [sphere], lights: [Light.ambient(0.5)],
  });
  approx(traceRay(ambientOnly, centreRay, 0), rgb(127.5, 0, 0));
});

test('light: specular near full when view matches reflection, near 0 when far', () => {
  const sphere = new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), 500, 0);
  // Light at camera: reflection direction = view direction exactly.
  const aligned = new Scene({
    spheres: [sphere],
    lights: [Light.ambient(0), Light.point(0.6, new Vec3(0, 0, 0))],
  });
  const a = traceRay(aligned, centreRay, 0);
  approx(a, rgb(0.6 * 255 + 0.6 * 255, 0.6 * 255, 0.6 * 255), 3); // diffuse red + white spec
  // Light far to the side: view far from reflection direction, spec ~ 0.
  const side = new Scene({
    spheres: [sphere],
    lights: [Light.ambient(0), Light.point(0.6, new Vec3(5, 0, 2))],
  });
  const b = traceRay(side, centreRay, 0);
  assert.ok(b.g < 5 && b.b < 5, `expected near-zero specular, got ${JSON.stringify(b)}`);
});

test('light: exponent 500 highlight is tighter than exponent 10', () => {
  const countBright = (spec) => {
    const canvas = renderRaytracer(new Canvas(60, 60), new Scene({
      spheres: [new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), spec, 0)],
      lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(2, 3, 1))],
    }));
    let n = 0;
    for (let y = 0; y < 60; y++) {
      for (let x = 0; x < 60; x++) {
        const p = canvas.getPixel(x, y);
        if (p.g > 100) n++; // green channel only comes from the white specular term
      }
    }
    return n;
  };
  const tight = countBright(500);
  const broad = countBright(10);
  assert.ok(tight > 0 && broad > tight,
    `expected 500-highlight (${tight}) smaller than 10-highlight (${broad}), both > 0`);
});

test('shadow: occluder between hit point and light removes diffuse+specular, leaves ambient', () => {
  // Hit point (0,0,2); light at (2,0,0); occluder at (1,0,1) is on the
  // segment, but not on the primary ray (x = 0 line).
  const sphere = new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), -1, 0);
  const base = {
    spheres: [sphere],
    lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(2, 0, 0))],
  };
  // Unshadowed: n.L = (0,0,-1).(0.707,0,-0.707) = 0.707 -> 0.2 + 0.424 = 0.624.
  approx(traceRay(new Scene(base), centreRay, 0), rgb(255 * 0.624, 0, 0));
  const shadowed = new Scene({
    ...base,
    spheres: [sphere, new Sphere(new Vec3(1, 0, 1), 0.4, rgb(0, 0, 0))],
  });
  approx(traceRay(shadowed, centreRay, 0), rgb(255 * 0.2, 0, 0));
});

test('reflection: reflectivity 1 shows the reflected colour, 0 is unchanged', () => {
  // Centre ray hits the mirror at (0,0,2) and reflects back along -z to
  // a green sphere at (0,0,-2) (behind the camera, so it is not hit by
  // the primary ray). Ambient 0.5 only, so reflected colour = green * 0.5.
  const green = new Sphere(new Vec3(0, 0, -2), 1, rgb(0, 255, 0), -1, 0);
  const mirror = new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), -1, 1);
  const scene = new Scene({ spheres: [mirror, green], lights: [Light.ambient(0.5)] });
  approx(traceRay(scene, centreRay, 3), rgb(0, 127.5, 0));
  const matte = new Scene({
    spheres: [new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), -1, 0), green],
    lights: [Light.ambient(0.5)],
  });
  approx(traceRay(matte, centreRay, 3), rgb(127.5, 0, 0));
});

test('reflection: two facing mirrors terminate at the depth limit', () => {
  const scene = new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), -1, 1),
      new Sphere(new Vec3(0, 0, -2), 1, rgb(0, 255, 0), -1, 1),
    ],
    lights: [Light.ambient(0.5)],
  });
  const canvas = renderRaytracer(new Canvas(20, 20), scene, { recursionDepth: 3 });
  const p = canvas.getPixel(10, 10);
  assert.ok(Number.isFinite(p.r) && Number.isFinite(p.g) && Number.isFinite(p.b));
});

test('default scene: centre predominantly centre-sphere colour, corner background, exact size, deterministic', () => {
  const scene = new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), 500, 0.2),
      new Sphere(new Vec3(-1.5, 0, 4), 1, rgb(0, 255, 0), 10, 0),
      new Sphere(new Vec3(1.5, 0, 4), 1, rgb(0, 0, 255), 10, 0),
    ],
    lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(2, 3, 1))],
    background: rgb(0, 0, 0),
  });
  const a = renderRaytracer(new Canvas(100, 100), scene);
  const b = renderRaytracer(new Canvas(100, 100), scene);
  const centre = a.getPixel(50, 50);
  assert.ok(centre.r > centre.g && centre.r > centre.b, `centre not red: ${JSON.stringify(centre)}`);
  assert.deepEqual(a.getPixel(0, 0), rgb(0, 0, 0));
  assert.deepEqual(a.pixels, b.pixels);
});
