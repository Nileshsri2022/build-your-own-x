package com.renderer;

/**
 * Camera plus image-plane corners (SPEC lesson 1).
 * x1 = top-left, x2 = top-right, x3 = bottom-left, x4 = bottom-right.
 * A ray starts at the plane point, with direction planePoint - camera.
 */
public final class Camera {
    public final Vec3 position;
    private final Vec3 x1;
    private final Vec3 x2;
    private final Vec3 x3;
    private final Vec3 x4;

    public Camera(Vec3 position, Vec3 x1, Vec3 x2, Vec3 x3, Vec3 x4) {
        this.position = position;
        this.x1 = x1;
        this.x2 = x2;
        this.x3 = x3;
        this.x4 = x4;
    }

    /** Ray through the plane point at fractions alpha (horizontal) and beta (vertical). */
    public Ray rayForPixel(double alpha, double beta) {
        Vec3 top = Vec3.lerp(x1, x2, alpha);
        Vec3 bottom = Vec3.lerp(x3, x4, alpha);
        Vec3 p = Vec3.lerp(top, bottom, beta);
        return new Ray(p, p.sub(position));
    }

    /** Ray through the centre of pixel (i, j) of a width x height image. */
    public Ray rayForPixel(int i, int j, int width, int height) {
        double alpha = (i + 0.5) / width;
        double beta = (j + 0.5) / height;
        return rayForPixel(alpha, beta);
    }
}
