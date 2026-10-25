// Camera: position + orientation (yaw around Y, pitch around X), looking
// along +z when both are 0 — the same forward direction as the raytracer.
// The view matrix is the inverse of the camera transform: R^T * T(-pos).
// Projection: x_screen = cx + (x * d / z) * (cw / vw),
//             y_screen = cy - (y * d / z) * (ch / vh),  z = view-space depth.
const { Mat4 } = require('../math');

class Camera {
  constructor({
    position = { x: 0, y: 0, z: 0 }, rotationX = 0, rotationY = 0,
    viewportDistance = 1, viewportWidth = 1, viewportHeight = 1, near = 0.1,
  } = {}) {
    this.position = position;
    this.rotationX = rotationX;
    this.rotationY = rotationY;
    this.viewportDistance = viewportDistance;
    this.viewportWidth = viewportWidth;
    this.viewportHeight = viewportHeight;
    this.near = near;
  }
  getViewMatrix() {
    const rot = Mat4.rotationY(this.rotationY).multiply(Mat4.rotationX(this.rotationX));
    const invRot = rot.transposeRotation();
    const invTrans = Mat4.translation(-this.position.x, -this.position.y, -this.position.z);
    return invRot.multiply(invTrans);
  }
  project(viewPoint, canvasWidth, canvasHeight) {
    if (viewPoint.z <= 0) return null;
    const sx = canvasWidth / 2
      + (viewPoint.x * this.viewportDistance / viewPoint.z)
        * (canvasWidth / this.viewportWidth);
    const sy = canvasHeight / 2
      - (viewPoint.y * this.viewportDistance / viewPoint.z)
        * (canvasHeight / this.viewportHeight);
    return { x: sx, y: sy };
  }
}
module.exports = { Camera };
