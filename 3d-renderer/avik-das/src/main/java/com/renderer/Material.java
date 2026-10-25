package com.renderer;

/** Surface material: colour, shininess (Phong power), reflectivity k in [0,1]. */
public final class Material {
    public final Color color;
    public final double shininess;
    public final double reflectivity;

    public Material(Color color, double shininess, double reflectivity) {
        if (reflectivity < 0.0 || reflectivity > 1.0) {
            throw new IllegalArgumentException("reflectivity must be in [0,1]");
        }
        this.color = color;
        this.shininess = shininess;
        this.reflectivity = reflectivity;
    }
}
