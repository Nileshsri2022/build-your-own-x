// Sphere: centre, radius, colour, specular exponent, reflectivity in [0,1].
class Sphere {
  constructor(center, radius, color, specular = -1, reflective = 0) {
    this.center = center;
    this.radius = radius;
    this.color = color;
    this.specular = specular; // -1 = matte (no specular term)
    this.reflective = reflective;
  }
  // Nearest intersection with t in [tMin, tMax], or null.
  intersect(ray, tMin, tMax) {
    const oc = ray.origin.sub(this.center);
    const a = ray.direction.dot(ray.direction); // 1 for a normalized ray
    const b = 2 * oc.dot(ray.direction);
    const c = oc.dot(oc) - this.radius * this.radius;
    const disc = b * b - 4 * a * c;
    if (disc < 0) return null;
    const root = Math.sqrt(disc);
    for (const t of [(-b - root) / (2 * a), (-b + root) / (2 * a)]) {
      if (t >= tMin && t <= tMax) {
        const point = ray.at(t);
        return { t, point, normal: point.sub(this.center).scale(1 / this.radius), sphere: this };
      }
    }
    return null;
  }
}
module.exports = { Sphere };
