// Textures: a grid of colours, sampled by UV with clamping.
// interpolateUv implements BOTH modes, so tests can distinguish them:
//   'perspective' — correct: interpolate u/z, v/z and 1/z, then divide
//   'linear'      — screen-space linear interpolation (the wrong contrast)
// FLAGGED, UNVERIFIED AGAINST CHAPTER 14: the exact interpolation detail
// (which attributes ride with the vertices, how the demo toggles) rests
// on the Introduction overview / demo excerpts. What is certain from the
// docs is the requirement itself: perspective-correct via reciprocal
// depth, visibly different from linear on a receding surface.
class Texture {
  constructor(width, height, pixels) {
    this.width = width;
    this.height = height;
    this.pixels = pixels; // rows of {r,g,b}, row 0 = v = 0
  }
  sample(u, v) {
    const x = Math.max(0, Math.min(this.width - 1, Math.floor(u * this.width)));
    const y = Math.max(0, Math.min(this.height - 1, Math.floor(v * this.height)));
    return this.pixels[y][x];
  }
  static checker(size = 4) {
    const pixels = [];
    for (let y = 0; y < size; y++) {
      const row = [];
      for (let x = 0; x < size; x++) {
        const white = (x + y) % 2 === 0;
        row.push(white ? { r: 255, g: 255, b: 255 } : { r: 0, g: 0, b: 0 });
      }
      pixels.push(row);
    }
    return new Texture(size, size, pixels);
  }
  // weights: barycentric weights at the pixel; depths: view-space z per
  // vertex; uvs: per-vertex {u,v}. Returns {u, v}.
  static interpolateUv(weights, depths, uvs, mode = 'perspective') {
    if (mode === 'linear') {
      let u = 0, v = 0;
      for (let i = 0; i < 3; i++) { u += weights[i] * uvs[i].u; v += weights[i] * uvs[i].v; }
      return { u, v };
    }
    if (mode !== 'perspective') throw new Error(`unknown texture interpolation mode: ${mode}`);
    let uz = 0, vz = 0, iz = 0;
    for (let i = 0; i < 3; i++) {
      const w = weights[i] / depths[i];
      uz += w * uvs[i].u;
      vz += w * uvs[i].v;
      iz += w;
    }
    return { u: uz / iz, v: vz / iz };
  }
}
module.exports = { Texture };
