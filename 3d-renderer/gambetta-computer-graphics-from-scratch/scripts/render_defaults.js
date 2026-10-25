// Renders both default scenes (ours, not the book's exact demo scenes —
// see SPEC section 5) and prints pixel summaries, for eyeballing.
// Not a browser run: both renderers draw into the in-memory Canvas.
const { Canvas } = require('../src/canvas');
const { Vec3 } = require('../src/math');
const { rgb } = require('../src/color');
const { Sphere } = require('../src/raytracer/sphere');
const { Light } = require('../src/raytracer/light');
const { Scene } = require('../src/raytracer/scene');
const { renderRaytracer } = require('../src/raytracer/render');
const { makeCube } = require('../src/rasterizer/model');
const { Instance } = require('../src/rasterizer/instance');
const { Camera } = require('../src/rasterizer/camera');
const { renderRasterizer } = require('../src/rasterizer/render');

function summarize(name, canvas) {
  const counts = {};
  for (let y = 0; y < canvas.height; y++) {
    for (let x = 0; x < canvas.width; x++) {
      const p = canvas.getPixel(x, y);
      const k = `${p.r},${p.g},${p.b}`;
      counts[k] = (counts[k] || 0) + 1;
    }
  }
  const top = Object.entries(counts).sort((a, b) => b[1] - a[1]).slice(0, 6);
  console.log(`${name} ${canvas.width}x${canvas.height}`);
  console.log(`  centre: ${JSON.stringify(canvas.getPixel(canvas.width >> 1, canvas.height >> 1))}`);
  console.log(`  corner: ${JSON.stringify(canvas.getPixel(0, 0))}`);
  console.log(`  top colours (rgb: count): ${top.map(([k, n]) => `${k}: ${n}`).join(', ')}`);
}

function main() {
  const rayScene = new Scene({
    spheres: [
      new Sphere(new Vec3(0, 0, 3), 1, rgb(255, 0, 0), 500, 0.2),
      new Sphere(new Vec3(-1.5, 0, 4), 1, rgb(0, 255, 0), 10, 0),
      new Sphere(new Vec3(1.5, 0, 4), 1, rgb(0, 0, 255), 10, 0),
    ],
    lights: [
      Light.ambient(0.2),
      Light.point(0.6, new Vec3(2, 3, 1)),
      Light.directional(0.2, new Vec3(0.3, -0.5, 1)),
    ],
    background: rgb(0, 0, 0),
  });
  summarize('raytracer', renderRaytracer(new Canvas(100, 100), rayScene));

  // Camera at -5 with a rotated cube: at -3 the front face would fill
  // the whole canvas, and face-on only one face would be visible.
  const camera = new Camera({ position: { x: 0, y: 0, z: -5 } });
  const lights = [
    Light.ambient(0.2),
    Light.directional(0.8, new Vec3(0.3, -0.5, 1)),
  ];
  const cube = new Instance(makeCube(), { specular: -1, rotationY: 0.6, rotationX: -0.4 });
  summarize('rasterizer', renderRasterizer(new Canvas(100, 100), [cube], camera, lights));
}
if (require.main === module) main();
module.exports = { main };
