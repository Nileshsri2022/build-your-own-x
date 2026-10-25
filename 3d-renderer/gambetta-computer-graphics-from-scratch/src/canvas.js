// Canvas: the only drawing primitive is PutPixel. In a browser this maps
// to a Canvas 2D context; here it is an in-memory pixel buffer with the
// identical interface, so pixel tests prove something about the same code
// the demo would call. Colours are 0-255 ints, clamped at PutPixel.
// Out-of-bounds PutPixel calls are ignored (never crash, never corrupt).
// Canvas space: x right, y down, origin at the top-left pixel; renderers
// convert their own centred coordinates to this space before calling.
class Canvas {
  constructor(width, height, background = { r: 0, g: 0, b: 0 }) {
    this.width = width;
    this.height = height;
    this.background = { ...background };
    this.pixels = new Uint8ClampedArray(width * height * 3);
    this.clear();
  }
  clear(color = this.background) {
    for (let i = 0; i < this.width * this.height; i++) {
      this.pixels[i * 3] = color.r;
      this.pixels[i * 3 + 1] = color.g;
      this.pixels[i * 3 + 2] = color.b;
    }
  }
  putPixel(x, y, color) {
    const ix = Math.round(x);
    const iy = Math.round(y);
    if (ix < 0 || iy < 0 || ix >= this.width || iy >= this.height) return;
    const i = (iy * this.width + ix) * 3;
    this.pixels[i] = clamp255(color.r);
    this.pixels[i + 1] = clamp255(color.g);
    this.pixels[i + 2] = clamp255(color.b);
  }
  getPixel(x, y) {
    if (x < 0 || y < 0 || x >= this.width || y >= this.height) return null;
    const i = (y * this.width + x) * 3;
    return { r: this.pixels[i], g: this.pixels[i + 1], b: this.pixels[i + 2] };
  }
}
function clamp255(v) {
  return Math.max(0, Math.min(255, Math.round(v)));
}
module.exports = { Canvas };
