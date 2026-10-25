package com.rogach;

/** Immutable 3D point: x, y, z. All math in double. */
public final class Vertex {
    public final double x;
    public final double y;
    public final double z;

    public Vertex(double x, double y, double z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public Vertex minus(Vertex other) {
        return new Vertex(x - other.x, y - other.y, z - other.z);
    }

    public Vertex scaled(double factor) {
        return new Vertex(x * factor, y * factor, z * factor);
    }

    public double length() {
        return Math.sqrt(x * x + y * y + z * z);
    }
}
