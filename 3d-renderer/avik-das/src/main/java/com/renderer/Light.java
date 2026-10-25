package com.renderer;

/** A point light: position and colour. */
public final class Light {
    public final Vec3 position;
    public final Color color;

    public Light(Vec3 position, Color color) {
        this.position = position;
        this.color = color;
    }
}
