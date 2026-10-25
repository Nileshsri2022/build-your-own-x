package com.renderer;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/** A scene: spheres, lights, background colour, ambient intensity. */
public final class Scene {
    public final List<Sphere> spheres;
    public final List<Light> lights;
    public final Color background;
    public final double ambientIntensity;

    public Scene(List<Sphere> spheres, List<Light> lights, Color background, double ambientIntensity) {
        this.spheres = Collections.unmodifiableList(new ArrayList<>(spheres));
        this.lights = Collections.unmodifiableList(new ArrayList<>(lights));
        this.background = background;
        this.ambientIntensity = ambientIntensity;
    }

    /** Nearest hit across all spheres, or null. */
    public Sphere.Hit nearestHit(Ray ray) {
        Sphere.Hit nearest = null;
        for (Sphere sphere : spheres) {
            Sphere.Hit hit = sphere.intersect(ray);
            if (hit != null && (nearest == null || hit.distance < nearest.distance)) {
                nearest = hit;
            }
        }
        return nearest;
    }

    /** The default scene from SPEC section 3 (ours, not the tutorial repo's exact scene). */
    public static Scene defaultScene() {
        List<Sphere> spheres = new ArrayList<>();
        spheres.add(new Sphere(
                new Vec3(0, 0, 5), 1.0,
                new Material(new Color(1, 0, 0), 50, 0.0)));
        spheres.add(new Sphere(
                new Vec3(-1.5, 0, 6), 1.0,
                new Material(new Color(0, 1, 0), 50, 0.2)));
        spheres.add(new Sphere(
                new Vec3(1.5, 0, 6), 1.0,
                new Material(new Color(0, 0, 1), 50, 0.2)));
        List<Light> lights = new ArrayList<>();
        lights.add(new Light(new Vec3(-5, 5, 0), Color.WHITE));
        return new Scene(spheres, lights, Color.BLACK, 0.1);
    }

    /** The default camera from SPEC section 3: (0,0,0), plane at z=2, corners (+-1,+-1,2). */
    public static Camera defaultCamera() {
        return new Camera(
                new Vec3(0, 0, 0),
                new Vec3(-1, 1, 2),
                new Vec3(1, 1, 2),
                new Vec3(-1, -1, 2),
                new Vec3(1, -1, 2));
    }
}
