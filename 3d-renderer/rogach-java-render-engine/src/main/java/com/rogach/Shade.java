package com.rogach;

import java.awt.Color;

/**
 * Flat shading: one shade per triangle.
 * Normal = normalised cross product of two edge vectors.
 * Directional light is the fixed vector (0, 0, 1) (behind the camera,
 * at infinite distance). Shade coefficient = |normal.z|; the sign is
 * dropped, so front and back faces shade alike (no back-face culling).
 * Colours are sRGB: convert to linear (power 2.4 approximation),
 * multiply by the coefficient, convert back (power 1/2.4).
 * The 2.4 exponent is an approximation, not the full sRGB spec.
 */
public final class Shade {
    private Shade() {
    }

    public static Vertex normal(Vertex v1, Vertex v2, Vertex v3) {
        Vertex a = v2.minus(v1);
        Vertex b = v3.minus(v1);
        Vertex cross = new Vertex(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
        double len = cross.length();
        if (len == 0) {
            return new Vertex(0, 0, 0);
        }
        return cross.scaled(1.0 / len);
    }

    public static double coefficient(Vertex unitNormal) {
        return Math.abs(unitNormal.z);
    }

    public static Color getShade(Color color, double coefficient) {
        double c = Math.max(0, Math.min(1, coefficient));
        return new Color(shadeChannel(color.getRed(), c),
                shadeChannel(color.getGreen(), c),
                shadeChannel(color.getBlue(), c));
    }

    private static int shadeChannel(int value, double coefficient) {
        double linear = Math.pow(value / 255.0, 2.4) * coefficient;
        int out = (int) Math.round(Math.pow(linear, 1.0 / 2.4) * 255.0);
        return Math.max(0, Math.min(255, out));
    }
}
