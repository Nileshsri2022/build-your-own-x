package com.rogach;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.util.List;

/**
 * Software renderer. Touches no Swing and opens no window: it returns
 * a BufferedImage, so tests can inspect pixels headlessly.
 * Orthographic projection: after rotation, z is discarded for drawing
 * (kept only for the z-buffer) and x/y are shifted to the screen centre.
 * Depth increases toward the viewer, so the greatest depth wins.
 * Heading is rotation in the XZ plane (about y), pitch rotation in
 * the YZ plane (about x); the combined matrix is computed once per
 * render, then applied to every vertex.
 */
public final class Renderer {
    private Renderer() {
    }

    public static BufferedImage render(List<Triangle> triangles,
            double headingDeg, double pitchDeg, int width, int height) {
        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException("width and height must be positive");
        }
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        double[] zBuffer = new double[width * height];
        for (int i = 0; i < zBuffer.length; i++) {
            zBuffer[i] = Double.NEGATIVE_INFINITY;
        }
        Matrix3 transform = Matrix3.rotationXZ(headingDeg)
                .multiply(Matrix3.rotationYZ(pitchDeg));
        for (Triangle t : triangles) {
            Vertex v1 = shift(transform.transform(t.v1), width, height);
            Vertex v2 = shift(transform.transform(t.v2), width, height);
            Vertex v3 = shift(transform.transform(t.v3), width, height);
            Vertex n = Shade.normal(v1, v2, v3);
            Color shaded = Shade.getShade(t.color, Shade.coefficient(n));
            double denom = (v2.y - v3.y) * (v1.x - v3.x)
                    + (v3.x - v2.x) * (v1.y - v3.y);
            if (denom == 0) {
                continue; // degenerate triangle: skip, never divide by zero
            }
            int minX = (int) Math.max(0, Math.floor(Math.min(v1.x, Math.min(v2.x, v3.x))));
            int maxX = (int) Math.min(width - 1, Math.ceil(Math.max(v1.x, Math.max(v2.x, v3.x))));
            int minY = (int) Math.max(0, Math.floor(Math.min(v1.y, Math.min(v2.y, v3.y))));
            int maxY = (int) Math.min(height - 1, Math.ceil(Math.max(v1.y, Math.max(v2.y, v3.y))));
            int rgb = shaded.getRGB();
            for (int y = minY; y <= maxY; y++) {
                for (int x = minX; x <= maxX; x++) {
                    double b1 = ((v2.y - v3.y) * (x - v3.x)
                            + (v3.x - v2.x) * (y - v3.y)) / denom;
                    double b2 = ((v3.y - v1.y) * (x - v3.x)
                            + (v1.x - v3.x) * (y - v3.y)) / denom;
                    double b3 = 1 - b1 - b2;
                    if (b1 < 0 || b1 > 1 || b2 < 0 || b2 > 1 || b3 < 0 || b3 > 1) {
                        continue;
                    }
                    double depth = b1 * v1.z + b2 * v2.z + b3 * v3.z;
                    int idx = y * width + x;
                    if (zBuffer[idx] < depth) {
                        image.setRGB(x, y, rgb);
                        zBuffer[idx] = depth;
                    }
                }
            }
        }
        return image;
    }

    /** Wireframe stepping stone (T3): triangle outlines only, no fill, no z-buffer. */
    public static BufferedImage renderWireframe(List<Triangle> triangles,
            double headingDeg, double pitchDeg, int width, int height) {
        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException("width and height must be positive");
        }
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        Matrix3 transform = Matrix3.rotationXZ(headingDeg)
                .multiply(Matrix3.rotationYZ(pitchDeg));
        for (Triangle t : triangles) {
            Vertex v1 = shift(transform.transform(t.v1), width, height);
            Vertex v2 = shift(transform.transform(t.v2), width, height);
            Vertex v3 = shift(transform.transform(t.v3), width, height);
            drawLine(image, v1, v2, t.color.getRGB(), width, height);
            drawLine(image, v2, v3, t.color.getRGB(), width, height);
            drawLine(image, v3, v1, t.color.getRGB(), width, height);
        }
        return image;
    }

    private static Vertex shift(Vertex v, int width, int height) {
        return new Vertex(v.x + width / 2.0, v.y + height / 2.0, v.z);
    }

    private static void drawLine(BufferedImage image, Vertex a, Vertex b,
            int rgb, int width, int height) {
        int steps = (int) Math.max(Math.abs(b.x - a.x), Math.abs(b.y - a.y));
        if (steps == 0) {
            plot(image, (int) Math.round(a.x), (int) Math.round(a.y), rgb, width, height);
            return;
        }
        for (int i = 0; i <= steps; i++) {
            double t = (double) i / steps;
            plot(image, (int) Math.round(a.x + (b.x - a.x) * t),
                    (int) Math.round(a.y + (b.y - a.y) * t), rgb, width, height);
        }
    }

    private static void plot(BufferedImage image, int x, int y, int rgb, int width, int height) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            image.setRGB(x, y, rgb);
        }
    }
}
