class Ray {
  constructor(origin, direction) {
    this.origin = origin;
    this.direction = direction.normalize();
  }
  at(t) { return this.origin.add(this.direction.scale(t)); }
}
module.exports = { Ray };
