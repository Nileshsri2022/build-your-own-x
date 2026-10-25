package com.renderer;

/** Immutable 3D vector. All math in double. */
public final class Vec3 {
    public final double x;
    public final double y;
    public final double z;

    public Vec3(double x, double y, double z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public Vec3 add(Vec3 o) {
        return new Vec3(x + o.x, y + o.y, z + o.z);
    }

    public Vec3 sub(Vec3 o) {
        return new Vec3(x - o.x, y - o.y, z - o.z);
    }

    public Vec3 scale(double s) {
        return new Vec3(x * s, y * s, z * s);
    }

    public double dot(Vec3 o) {
        return x * o.x + y * o.y + z * o.z;
    }

    public Vec3 cross(Vec3 o) {
        return new Vec3(
                y * o.z - z * o.y,
                z * o.x - x * o.z,
                x * o.y - y * o.x);
    }

    public double length() {
        return Math.sqrt(dot(this));
    }

    /**
     * Unit vector in the same direction.
     * Zero vector is an error: normalising it would silently produce NaN.
     */
    public Vec3 normalize() {
        double len = length();
        if (len == 0.0) {
            throw new IllegalArgumentException("cannot normalize the zero vector");
        }
        return scale(1.0 / len);
    }

    /** Linear interpolation: (1-t)*a + t*b. */
    public static Vec3 lerp(Vec3 a, Vec3 b, double t) {
        return a.scale(1.0 - t).add(b.scale(t));
    }
}
