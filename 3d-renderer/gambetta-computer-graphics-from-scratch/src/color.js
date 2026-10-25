// Colour arithmetic. Internally colours are floats in 0-255 per channel;
// clamping happens once, at PutPixel (see canvas.js), never silently inside
// arithmetic, so a test can see an unclamped intermediate if it asks.
function rgb(r, g, b) { return { r, g, b }; }
function add(a, b) { return rgb(a.r + b.r, a.g + b.g, a.b + b.b); }
function scale(c, k) { return rgb(c.r * k, c.g * k, c.b * k); }
// Component-wise multiply, in 0-255 space: white (255,255,255) is identity.
function multiply(a, b) {
  return rgb((a.r * b.r) / 255, (a.g * b.g) / 255, (a.b * b.b) / 255);
}
function clamp(c) {
  return rgb(
    Math.max(0, Math.min(255, c.r)),
    Math.max(0, Math.min(255, c.g)),
    Math.max(0, Math.min(255, c.b)),
  );
}
module.exports = { rgb, add, scale, multiply, clamp };
