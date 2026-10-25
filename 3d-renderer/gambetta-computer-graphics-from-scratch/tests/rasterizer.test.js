// Part II tests (TEST_PLAN section 4) plus the comparison test (section 5).
const test = require('node:test');
const assert = require('node:assert/strict');
const { Canvas } = require('../src/canvas');
const { Vec3 } = require('../src/math');
const { rgb } = require('../src/color');
const { Model, makeCube } = require('../src/rasterizer/model');
const { Instance } = require('../src/rasterizer/instance');
const { Camera } = require('../src/rasterizer/camera');
const { Light } = require('../src/raytracer/light');
const { clipPolygonAgainstNear } = require('../src/rasterizer/clip');
const { makeDepthBuffer, drawLine, rasterizeTriangle } = require('../src/rasterizer/raster');
const { Texture } = require('../src/rasterizer/texture');
const { renderRasterizer } = require('../src/rasterizer/render');
const { Sphere } = require('../src/raytracer/sphere');
const { Scene } = require('../src/raytracer/scene');
const { renderRaytracer } = require('../src/raytracer/render');

const CAMERA = () => new Camera({ position: { x: 0, y: 0, z: -5 } });
// Rotated cube: face-on shows only one face; rotation shows three.
const ROT_CUBE = () => new Instance(makeCube(), { specular: -1, rotationY: 0.6, rotationX: -0.4 });
const LIGHTS = () => [Light.ambient(0.2), Light.directional(0.8, new Vec3(0.3, -0.5, 1))];
const countNonBg = (canvas, bg = rgb(0, 0, 0)) => {
  let n = 0;
  for (let y = 0; y < canvas.height; y++) {
    for (let x = 0; x < canvas.width; x++) {
      const p = canvas.getPixel(x, y);
      if (p.r !== bg.r || p.g !== bg.g || p.b !== bg.b) n++;
    }
  }
  return n;
};
const distinctColors = (canvas) => {
  const s = new Set();
  for (let y = 0; y < canvas.height; y++) {
    for (let x = 0; x < canvas.width; x++) {
      const p = canvas.getPixel(x, y);
      s.add(`${p.r},${p.g},${p.b}`);
    }
  }
  return s;
};

test('model: every cube triangle normal points outward', () => {
  const cube = makeCube();
  for (const tri of cube.triangles) {
    const n = cube.faceNormal(tri);
    const centroid = tri.indices
      .map((i) => cube.vertices[i])
      .reduce((a, v) => a.add(v), new Vec3(0, 0, 0)).scale(1 / 3);
    assert.ok(n.dot(centroid) > 0, `triangle normal not outward: ${JSON.stringify(n)}`);
  }
  assert.equal(cube.triangles.length, 12);
});

test('lines: horizontal, vertical, diagonal include both endpoints', () => {
  const c = new Canvas(10, 10);
  drawLine(c, 1, 5, 8, 5, rgb(255, 0, 0));
  drawLine(c, 5, 1, 5, 8, rgb(0, 255, 0));
  drawLine(c, 0, 0, 9, 9, rgb(0, 0, 255));
  assert.deepEqual(c.getPixel(1, 5), rgb(255, 0, 0));
  assert.deepEqual(c.getPixel(8, 5), rgb(255, 0, 0));
  assert.deepEqual(c.getPixel(5, 1), rgb(0, 255, 0));
  assert.deepEqual(c.getPixel(5, 8), rgb(0, 255, 0));
  assert.deepEqual(c.getPixel(0, 0), rgb(0, 0, 255));
  assert.deepEqual(c.getPixel(9, 9), rgb(0, 0, 255));
});

test('filled triangle: interior coloured, exterior not, shared edge consistent', () => {
  const c = new Canvas(20, 20);
  const db = makeDepthBuffer(20, 20);
  const tri = (a, b, d, color) => rasterizeTriangle(c, db,
    [a, b, d].map(([x, y]) => ({ x, y, z: 2 })), {
      shading: 'flat', flatColor: color, lights: [], baseColor: color, specular: -1,
    });
  // Quad (2,2)-(18,2)-(18,18)-(2,18) split on the diagonal.
  tri([2, 2], [18, 2], [18, 18], rgb(255, 0, 0));
  tri([2, 2], [18, 18], [2, 18], rgb(0, 255, 0));
  // (5,5) lies on the shared diagonal; the first triangle wins it (strict
  // depth test rejects the equal-depth second draw), so it is red.
  assert.deepEqual(c.getPixel(5, 5), rgb(255, 0, 0));
  assert.ok(countNonBg(c) > 200, 'quad interior mostly coloured');
  assert.deepEqual(c.getPixel(0, 0), rgb(0, 0, 0));
  // Shared diagonal pixels are all one colour (no double-draw artefact
  // visible as a third colour) and never background.
  const diag = [];
  for (let i = 3; i < 17; i++) diag.push(c.getPixel(i, i));
  assert.ok(diag.every((p) => (p.r === 255 && p.g === 0) || (p.g === 255 && p.r === 0)),
    `shared edge has unexpected colours: ${JSON.stringify(diag)}`);
});

test('shaded triangle: vertex colours blend to the centre', () => {
  const c = new Canvas(30, 30);
  const db = makeDepthBuffer(30, 30);
  rasterizeTriangle(c, db, [
    { x: 5, y: 25, z: 2, color: rgb(255, 0, 0) },
    { x: 25, y: 25, z: 2, color: rgb(0, 255, 0) },
    { x: 15, y: 5, z: 2, color: rgb(0, 0, 255) },
  ], { shading: 'gouraud', lights: [], baseColor: rgb(255, 255, 255), specular: -1 });
  const centre = c.getPixel(15, 18); // near the centroid (15, 18.3)
  for (const k of ['r', 'g', 'b']) {
    assert.ok(Math.abs(centre[k] - 85) <= 25, `centre channel ${k} = ${centre[k]}, expected ~85`);
  }
  const nearRed = c.getPixel(7, 23);
  assert.ok(nearRed.r > 150 && nearRed.g < 110, `near-red vertex off: ${JSON.stringify(nearRed)}`);
});

test('clip: straddling polygon is cut to the near plane, behind polygon empties', () => {
  const v = (z) => ({ pos: new Vec3(0, 0, z) });
  const clipped = clipPolygonAgainstNear([v(0.05), v(2), v(2)], 0.1);
  assert.equal(clipped.length, 4);
  assert.ok(clipped.every((p) => p.pos.z >= 0.1 - 1e-9));
  assert.deepEqual(clipPolygonAgainstNear([v(0.01), v(0.02), v(0.05)], 0.1), []);
});

test('perspective: farther cube renders smaller, by roughly the expected ratio', () => {
  const renderAt = (camZ) => renderRasterizer(new Canvas(100, 100),
    [new Instance(makeCube(), { specular: -1 })],
    new Camera({ position: { x: 0, y: 0, z: camZ } }), LIGHTS());
  const near = countNonBg(renderAt(-5));
  const far = countNonBg(renderAt(-10));
  const ratio = far / near; // projected area scales as (distance ratio)^2 = 0.25
  assert.ok(ratio > 0.15 && ratio < 0.4, `area ratio ${ratio} (near ${near}, far ${far})`);
});

test('scene: default cube is recognisable — several face colours plus background', () => {
  const c = renderRasterizer(new Canvas(100, 100),
    [ROT_CUBE()], CAMERA(), LIGHTS());
  const colors = distinctColors(c);
  assert.ok(colors.has('0,0,0'), 'background present');
  assert.ok(colors.size >= 3, `expected several face colours, got ${[...colors].join(' | ')}`);
  assert.ok(countNonBg(c) > 1000, 'cube covers a substantial area');
});

test('clipping: cube behind the camera is invisible, no crash; straddling cube partly visible', () => {
  const behind = renderRasterizer(new Canvas(50, 50),
    [new Instance(makeCube(), { specular: -1 })],
    new Camera({ position: { x: 0, y: 0, z: 3 } }), LIGHTS());
  assert.equal(countNonBg(behind), 0);
  // Straddling cube: camera at (1.5, 0, -5), cube centre at (0, 0, -3.95),
  // so the cube's view-space z range (0.05..2.05) crosses the near plane
  // (0.1) while the camera stays OUTSIDE the cube. The camera is also
  // off-axis in x, because from straight ahead only the front face is
  // visible — and that face is entirely behind the near plane here, so
  // a straight-on camera would legitimately render nothing. Off-axis,
  // the +x side face is visible and its clipped polygon renders.
  const straddle = renderRasterizer(new Canvas(100, 100),
    [new Instance(makeCube(), { specular: -1, position: { x: 0, y: 0, z: -3.95 } })],
    new Camera({ position: { x: 1.5, y: 0, z: -5 } }), LIGHTS());
  assert.ok(countNonBg(straddle) > 0, 'straddling cube should show its visible part');
  // Triangle-level, separate from instance-level: one triangle straddling
  // the near plane renders; one fully behind does not. Winding is
  // front-facing (normal toward the camera).
  const triModel = new Model(
    [new Vec3(-2, -2, -1), new Vec3(2, -2, 2), new Vec3(0, 2, 2)],
    [{ indices: [0, 2, 1], color: rgb(255, 0, 0), uvs: null }],
  );
  const triCam = new Camera({ position: { x: 0, y: 0, z: -0.5 } });
  const triHit = renderRasterizer(new Canvas(50, 50),
    [new Instance(triModel, { specular: -1 })], triCam, LIGHTS());
  assert.ok(countNonBg(triHit) > 0);
  const triBehind = new Model(
    [new Vec3(-2, -2, -1), new Vec3(2, -2, -0.9), new Vec3(0, 2, -0.8)],
    [{ indices: [0, 2, 1], color: rgb(255, 0, 0), uvs: null }],
  );
  const triMiss = renderRasterizer(new Canvas(50, 50),
    [new Instance(triBehind, { specular: -1 })], triCam, LIGHTS());
  assert.equal(countNonBg(triMiss), 0);
});

test('hidden surfaces: nearer triangle wins regardless of draw order', () => {
  const mk = (z, color) => new Model(
    [new Vec3(-2, -2, 0), new Vec3(0, 2, 0), new Vec3(2, -2, 0)],
    [{ indices: [0, 1, 2], color, uvs: null }],
  );
  const cam = CAMERA();
  const near = new Instance(mk(0, rgb(255, 0, 0)), { specular: -1 });
  const far = new Instance(mk(3, rgb(0, 0, 255)), { specular: -1 });
  // Instances are placed by position offset: near at z=0, far at z=3.
  near.position = { x: 0, y: 0, z: 0 };
  far.position = { x: 0, y: 0, z: 3 };
  const a = renderRasterizer(new Canvas(50, 50), [far, near], cam, LIGHTS());
  const b = renderRasterizer(new Canvas(50, 50), [near, far], cam, LIGHTS());
  // Overlap pixel (centre): nearer (red, shaded) in both orders, and the
  // two renders agree there.
  assert.deepEqual(a.getPixel(25, 25), b.getPixel(25, 25));
  assert.ok(a.getPixel(25, 25).r > a.getPixel(25, 25).b, 'nearer triangle colour wins');
});

test('culling: toggle works — a reversed (back-facing) quad is culled only when on', () => {
  // Triangle wound so its normal points away from the camera (+z).
  const rev = new Model(
    [new Vec3(-2, -2, 0), new Vec3(2, -2, 0), new Vec3(0, 2, 0)],
    [{ indices: [0, 1, 2], color: rgb(255, 0, 0), uvs: null }],
  );
  const on = renderRasterizer(new Canvas(50, 50),
    [new Instance(rev, { specular: -1 })], CAMERA(), LIGHTS(), { culling: true });
  const off = renderRasterizer(new Canvas(50, 50),
    [new Instance(rev, { specular: -1 })], CAMERA(), LIGHTS(), { culling: false });
  assert.equal(countNonBg(on), 0, 'back-facing quad must be culled when on');
  assert.ok(countNonBg(off) > 0, 'back-facing quad must draw when off');
});

test('shading: flat, Gouraud and Phong differ on the same cube scene', () => {
  const renderMode = (mode) => renderRasterizer(new Canvas(80, 80),
    [new Instance(makeCube(), { specular: 50 })], CAMERA(), LIGHTS(), { shading: mode });
  const flat = renderMode('flat');
  const gouraud = renderMode('gouraud');
  const phong = renderMode('phong');
  const differs = (a, b) => {
    for (let i = 0; i < a.pixels.length; i++) if (a.pixels[i] !== b.pixels[i]) return true;
    return false;
  };
  assert.ok(differs(flat, gouraud), 'flat and Gouraud must differ');
  assert.ok(differs(gouraud, phong), 'Gouraud and Phong must differ');
  assert.ok(differs(flat, phong), 'flat and Phong must differ');
  assert.throws(() => renderRasterizer(new Canvas(10, 10),
    [new Instance(makeCube())], CAMERA(), LIGHTS(), { shading: 'nope' }), /unknown shading mode/);
});

test('shading: Phong highlight moves when the light moves', () => {
  const tri = new Model(
    [new Vec3(-3, -3, 0), new Vec3(0, 3, 0), new Vec3(3, -3, 0)],
    [{ indices: [0, 1, 2], color: rgb(200, 200, 200), uvs: null }],
  );
  // Vertex normals all face the camera, so the highlight is purely a
  // function of light position, not of vertex placement.
  tri.vertexNormals = tri.vertices.map(() => new Vec3(0, 0, -1));
  const brightestX = (lightPos) => {
    const c = renderRasterizer(new Canvas(60, 60),
      [new Instance(tri, { specular: 50 })], CAMERA(),
      [Light.ambient(0.1), Light.point(1, lightPos)], { shading: 'phong' });
    let best = -1, bestX = 0;
    for (let y = 0; y < 60; y++) {
      for (let x = 0; x < 60; x++) {
        const p = c.getPixel(x, y);
        const v = p.r + p.g + p.b;
        if (v > best) { best = v; bestX = x; }
      }
    }
    return bestX;
  };
  const left = brightestX(new Vec3(-2, 2, -2));
  const right = brightestX(new Vec3(2, 2, -2));
  assert.ok(right > left, `highlight must move right with the light: left ${left}, right ${right}`);
});

test('texture: perspective-correct interpolation is hand-verified and differs from linear', () => {
  // Barycentric weights [0.25, 0.25, 0.5], depths [2, 2, 6],
  // uvs [(0,0), (1,0), (0,1)].
  // Linear: u = 0.25, v = 0.5.
  // Perspective: weights/z = [0.125, 0.125, 0.0833], sum 0.3333 ->
  //   u = 0.125/0.3333 = 0.375, v = 0.0833/0.3333 = 0.25.
  const w = [0.25, 0.25, 0.5];
  const z = [2, 2, 6];
  const uvs = [{ u: 0, v: 0 }, { u: 1, v: 0 }, { u: 0, v: 1 }];
  const persp = Texture.interpolateUv(w, z, uvs, 'perspective');
  const lin = Texture.interpolateUv(w, z, uvs, 'linear');
  assert.ok(Math.abs(persp.u - 0.375) < 0.01 && Math.abs(persp.v - 0.25) < 0.01,
    `perspective uv ${JSON.stringify(persp)}`);
  assert.ok(Math.abs(lin.u - 0.25) < 0.01 && Math.abs(lin.v - 0.5) < 0.01,
    `linear uv ${JSON.stringify(lin)}`);
  assert.ok(Math.abs(persp.u - lin.u) > 0.1, 'modes must visibly differ');
});

test('texture: a receding quad renders differently under perspective vs linear', () => {
  // Two-column stripe texture: u < 0.5 black, u >= 0.5 white.
  const stripes = new Texture(2, 1, [[rgb(0, 0, 0), rgb(255, 255, 255)]]);
  const quad = new Model(
    [new Vec3(-2, -1, 2), new Vec3(2, -1, 2), new Vec3(1, 1, 6), new Vec3(-1, 1, 6)],
    [
      { indices: [0, 3, 2], color: rgb(255, 255, 255), uvs: [{ u: 0, v: 0 }, { u: 0, v: 1 }, { u: 1, v: 1 }] },
      { indices: [0, 2, 1], color: rgb(255, 255, 255), uvs: [{ u: 0, v: 0 }, { u: 1, v: 1 }, { u: 1, v: 0 }] },
    ],
  );
  const renderMode = (mode) => renderRasterizer(new Canvas(100, 100),
    [new Instance(quad, { specular: -1, texture: stripes })], CAMERA(),
    [Light.ambient(1)], { textureMode: mode });
  const persp = renderMode('perspective');
  const lin = renderMode('linear');
  let diff = 0;
  for (let i = 0; i < persp.pixels.length; i++) if (persp.pixels[i] !== lin.pixels[i]) diff++;
  assert.ok(diff > 0, 'perspective and linear texture renders must differ');
  assert.ok(countNonBg(persp) > 0 && countNonBg(lin) > 0);
});

test('pixel tests: default cube, exact size, face colours, background, deterministic', () => {
  const mk = () => renderRasterizer(new Canvas(100, 100),
    [ROT_CUBE()], CAMERA(), LIGHTS());
  const a = mk();
  const b = mk();
  assert.equal(a.width, 100);
  assert.equal(a.height, 100);
  assert.deepEqual(a.pixels, b.pixels);
  const colors = distinctColors(a);
  assert.ok(colors.has('0,0,0'));
  assert.ok(colors.size >= 3);
});

test('comparison: both renderers show a centred object over background, without pixel equality', () => {
  const ray = renderRaytracer(new Canvas(100, 100), new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), 500, 0.2),
      new Sphere(new Vec3(-1.5, 0, 4), 1, rgb(0, 255, 0), 10, 0),
      new Sphere(new Vec3(1.5, 0, 4), 1, rgb(0, 0, 255), 10, 0),
    ],
    lights: [Light.ambient(0.2), Light.point(0.6, new Vec3(2, 3, 1))],
  }));
  const rast = renderRasterizer(new Canvas(100, 100),
    [ROT_CUBE()], CAMERA(), LIGHTS());
  for (const c of [ray, rast]) {
    assert.ok(countNonBg(c) > 500, 'centred object present');
    assert.deepEqual(c.getPixel(0, 0), rgb(0, 0, 0), 'background corner present');
    const centre = c.getPixel(50, 50);
    assert.ok(centre.r !== 0 || centre.g !== 0 || centre.b !== 0, 'centre not background');
  }
  let same = true;
  for (let i = 0; i < ray.pixels.length; i++) if (ray.pixels[i] !== rast.pixels[i]) { same = false; break; }
  assert.ok(!same, 'the two renderers must NOT be pixel-identical (different algorithms)');
});
