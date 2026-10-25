package com.renderer;

/** A ray: an origin point plus a direction vector. */
public final class Ray {
    public final Vec3 origin;
    public final Vec3 direction;

    public Ray(Vec3 origin, Vec3 direction) {
        this.origin = origin;
        this.direction = direction;
    }

    /** Point on the ray at parameter t: origin + t * direction. */
    public Vec3 pointAt(double t) {
        return origin.add(direction.scale(t));
    }
}
