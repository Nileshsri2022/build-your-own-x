package com.rogach;

/**
 * 3x3 rotation matrix. Angles are in degrees.
 * rotationXY rotates in the XY plane (about z), rotationYZ in the YZ
 * plane (about x), rotationXZ in the XZ plane (about y).
 * multiply(other) returns this * other, i.e. applying other first,
 * then this — so heading.multiply(pitch) applies pitch, then heading.
 * A 3x3 matrix cannot express translation; screen-centre translation
 * is done separately by the renderer.
 */
public final class Matrix3 {
    private final double[][] m;

    public Matrix3(double[][] values) {
        m = new double[3][3];
        for (int i = 0; i < 3; i++) {
            System.arraycopy(values[i], 0, m[i], 0, 3);
        }
    }

    public static Matrix3 identity() {
        return new Matrix3(new double[][]{
                {1, 0, 0}, {0, 1, 0}, {0, 0, 1}});
    }

    public static Matrix3 rotationXY(double degrees) {
        double c = Math.cos(Math.toRadians(degrees));
        double s = Math.sin(Math.toRadians(degrees));
        return new Matrix3(new double[][]{
                {c, -s, 0}, {s, c, 0}, {0, 0, 1}});
    }

    public static Matrix3 rotationYZ(double degrees) {
        double c = Math.cos(Math.toRadians(degrees));
        double s = Math.sin(Math.toRadians(degrees));
        return new Matrix3(new double[][]{
                {1, 0, 0}, {0, c, -s}, {0, s, c}});
    }

    public static Matrix3 rotationXZ(double degrees) {
        double c = Math.cos(Math.toRadians(degrees));
        double s = Math.sin(Math.toRadians(degrees));
        return new Matrix3(new double[][]{
                {c, 0, s}, {0, 1, 0}, {-s, 0, c}});
    }

    public Matrix3 multiply(Matrix3 other) {
        double[][] r = new double[3][3];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                r[i][j] = m[i][0] * other.m[0][j]
                        + m[i][1] * other.m[1][j]
                        + m[i][2] * other.m[2][j];
            }
        }
        return new Matrix3(r);
    }

    public Vertex transform(Vertex v) {
        return new Vertex(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
    }
}
