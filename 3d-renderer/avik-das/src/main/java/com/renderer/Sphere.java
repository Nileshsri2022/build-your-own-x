package com.renderer;

/** A sphere: centre, radius, material. Intersection via the SPEC lesson-2 quadratic. */
public final class Sphere {
    /** Smallest distance counted as a hit; avoids self-intersection (SPEC/DESIGN epsilon). */
    public static final double EPSILON = 1e-4;

    public final Vec3 center;
    public final double radius;
    public final Material material;

    public Sphere(Vec3 center, double radius, Material material) {
        this.center = center;
        this.radius = radius;
        this.material = material;
    }

    /** A ray-sphere hit, or null when there is none. */
    public static final class Hit {
        public final Vec3 point;
        public final Vec3 normal;
        public final double distance;
        public final Sphere sphere;

        public Hit(Vec3 point, Vec3 normal, double distance, Sphere sphere) {
            this.point = point;
            this.normal = normal;
            this.distance = distance;
            this.sphere = sphere;
        }
    }

    /**
     * Nearest hit with t > EPSILON, or null.
     * a = d.d, b = 2*d.(o-C), c = (o-C).(o-C) - r^2, discriminant b^2 - 4*a*c.
     */
    public Hit intersect(Ray ray) {
        Vec3 oc = ray.origin.sub(center);
        double a = ray.direction.dot(ray.direction);
        double b = 2.0 * ray.direction.dot(oc);
        double c = oc.dot(oc) - radius * radius;
        double discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0.0) {
            return null;
        }
        double sqrtD = Math.sqrt(discriminant);
        double t = (-b - sqrtD) / (2.0 * a);
        if (t <= EPSILON) {
            t = (-b + sqrtD) / (2.0 * a);
        }
        if (t <= EPSILON) {
            return null;
        }
        Vec3 point = ray.pointAt(t);
        Vec3 normal = point.sub(center).scale(1.0 / radius).normalize();
        return new Hit(point, normal, t, this);
    }
}
