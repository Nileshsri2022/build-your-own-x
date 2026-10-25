// Demo harness (not engine code): renders the demo scenes to PPM files
// in out/ for eyeballing, as TEST_PLAN allows (frames captured by the
// harness, not an engine image-file feature). No browser/canvas run is
// claimed — this runs headlessly on Node.
import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import { Vector3 } from "./math.js";
import { Camera, cubeMesh, loadMeshFromJson, Texture } from "./mesh.js";
import { Device } from "./device.js";

const dir = new URL("../", import.meta.url).pathname;
mkdirSync(dir + "out", { recursive: true });
const camera = new Camera(new Vector3(0, 0, 10), new Vector3(0, 0, 0));
const light = new Vector3(0, 5, 10);

function shot(name, meshes, options) {
  const d = new Device(200, 200);
  d.render(camera, meshes, { light, ...options });
  writeFileSync(dir + "out/" + name + ".ppm", d.toPPM());
  const nonBg = countNonBg(d);
  console.log(name, "non-background pixels:", nonBg);
  return d;
}
function countNonBg(d) {
  let n = 0;
  for (let i = 0; i < d.width * d.height; i++) {
    if (d.backBuffer[i * 3] || d.backBuffer[i * 3 + 1] || d.backBuffer[i * 3 + 2]) n++;
  }
  return n;
}

const cube = cubeMesh();
shot("cube-wireframe", [cube], { mode: "wireframe" });
shot("cube-flat", [cube], { mode: "flat" });
shot("cube-gouraud", [cube], { mode: "gouraud" });

const standin = loadMeshFromJson(JSON.parse(readFileSync(dir + "fixtures/suzanne-standin.json", "utf8")));
shot("suzanne-standin-flat", [standin], { mode: "flat" });

const torus = loadMeshFromJson(JSON.parse(readFileSync(dir + "fixtures/torus.json", "utf8")));
torus.texture = Texture.checker(4, 64);
torus.color = [1, 1, 1];
shot("torus-textured-gouraud", [torus], { mode: "gouraud" });

// Deterministic animation: fixed rotation step per frame, frame N
// must reproduce exactly (checked in tests too).
const rotating = cubeMesh();
rotating.rotation = new Vector3(0, 0.3, 0);
shot("cube-rotated-frame", [rotating], { mode: "gouraud" });
console.log("frames written to out/");
