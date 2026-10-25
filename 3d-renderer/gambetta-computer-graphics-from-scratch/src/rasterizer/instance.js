// Instance: a Model placed in the scene. Transform = T * Ry * Rx * S
// (scale first, then rotation, then translation).
const { Mat4 } = require('../math');

class Instance {
  constructor(model, {
    position = { x: 0, y: 0, z: 0 }, rotationX = 0, rotationY = 0,
    scale = 1, color = null, specular = -1, texture = null, shading = 'flat',
  } = {}) {
    this.model = model;
    this.position = position;
    this.rotationX = rotationX;
    this.rotationY = rotationY;
    this.scale = scale;
    this.color = color; // optional override of per-face colours
    this.specular = specular;
    this.texture = texture;
    this.shading = shading;
  }
  getTransform() {
    const t = Mat4.translation(this.position.x, this.position.y, this.position.z);
    const ry = Mat4.rotationY(this.rotationY);
    const rx = Mat4.rotationX(this.rotationX);
    const s = Mat4.scaling(this.scale, this.scale, this.scale);
    return t.multiply(ry).multiply(rx).multiply(s);
  }
  // Rotation part only, for transforming normals (uniform scale assumed).
  getRotationMatrix() {
    return Mat4.rotationY(this.rotationY).multiply(Mat4.rotationX(this.rotationX));
  }
}
module.exports = { Instance };
