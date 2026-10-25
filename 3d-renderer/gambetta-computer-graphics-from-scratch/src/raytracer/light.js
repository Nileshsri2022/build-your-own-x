// Light types: 'ambient' (intensity only), 'point' (position, no falloff),
// 'directional' (direction = direction the light TRAVELS; the vector
// toward the light is its negation). All lights are white; intensity is
// a scalar. Unknown types throw — they must fail loudly, not render dark.
class Light {
  constructor(type, intensity, position = null, direction = null) {
    if (!['ambient', 'point', 'directional'].includes(type)) {
      throw new Error(`unknown light type: ${type}`);
    }
    this.type = type;
    this.intensity = intensity;
    this.position = position;
    this.direction = direction;
  }
  static ambient(intensity) { return new Light('ambient', intensity); }
  static point(intensity, position) { return new Light('point', intensity, position); }
  static directional(intensity, direction) {
    return new Light('directional', intensity, null, direction);
  }
}
module.exports = { Light };
