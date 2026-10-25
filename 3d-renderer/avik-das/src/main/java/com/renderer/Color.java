package com.renderer;

/** RGB colour with each channel in [0,1] before clamping. */
public final class Color {
    public final double r;
    public final double g;
    public final double b;

    public static final Color BLACK = new Color(0, 0, 0);
    public static final Color WHITE = new Color(1, 1, 1);

    public Color(double r, double g, double b) {
        this.r = r;
        this.g = g;
        this.b = b;
    }

    public Color add(Color o) {
        return new Color(r + o.r, g + o.g, b + o.b);
    }

    public Color scale(double s) {
        return new Color(r * s, g * s, b * s);
    }

    /** Per-channel multiplication. */
    public Color multiply(Color o) {
        return new Color(r * o.r, g * o.g, b * o.b);
    }

    public Color clamp() {
        return new Color(
                Math.min(1.0, Math.max(0.0, r)),
                Math.min(1.0, Math.max(0.0, g)),
                Math.min(1.0, Math.max(0.0, b)));
    }

    /** Packed 0xRRGGBB int: each channel clamped to [0,1], then round(channel * 255). */
    public int toRgbInt() {
        Color c = clamp();
        int ri = (int) Math.round(c.r * 255);
        int gi = (int) Math.round(c.g * 255);
        int bi = (int) Math.round(c.b * 255);
        return (ri << 16) | (gi << 8) | bi;
    }
}
