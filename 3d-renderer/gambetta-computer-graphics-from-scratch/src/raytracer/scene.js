class Scene {
  constructor({ spheres = [], lights = [], background = { r: 0, g: 0, b: 0 } } = {}) {
    this.spheres = spheres;
    this.lights = lights;
    this.background = background;
  }
}
module.exports = { Scene };
