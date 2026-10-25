package com.renderer;

import java.awt.image.BufferedImage;
import java.util.Random;

/**
 * Backward ray tracer (SPEC lessons 1-6): Phong shading, hard shadows,
 * recursive reflections to depth 3, seeded jittered anti-aliasing.
 */
public final class Renderer {
    public static final int MAX_DEPTH = 3;
    public static final long DEFAULT_SEED = 42L;

    private final Scene scene;
    private final Camera camera;

    public Renderer(Scene scene, Camera camera) {
        this.scene = scene;
        this.camera = camera;
    }

    /** Reflect vector d about unit normal n: d - 2*(d.n)*n. */
    public static Vec3 reflect(Vec3 d, Vec3 n) {
        return d.sub(n.scale(2.0 * d.dot(n)));
    }

    /** Render to a BufferedImage. samples=1 uses exact pixel centres (no jitter). */
    public BufferedImage render(int width, int height, int samples) {
        return render(width, height, samples, DEFAULT_SEED);
    }

    public BufferedImage render(int width, int height, int samples, long seed) {
        if (width <= 0 || height <= 0 || samples <= 0) {
            throw new IllegalArgumentException("width, height and samples must be positive");
        }
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
        Random random = new Random(seed);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                Color colour = Color.BLACK;
                for (int s = 0; s < samples; s++) {
                    Ray ray;
                    if (samples == 1) {
                        ray = camera.rayForPixel(i, j, width, height);
                    } else {
                        double alpha = (i + random.nextDouble()) / width;
                        double beta = (j + random.nextDouble()) / height;
                        ray = camera.rayForPixel(alpha, beta);
                    }
                    colour = colour.add(trace(ray, 0));
                }
                image.setRGB(i, j, colour.scale(1.0 / samples).toRgbInt());
            }
        }
        return image;
    }

    /** Trace one ray, recursively for reflections. */
    public Color trace(Ray ray, int depth) {
        Sphere.Hit hit = scene.nearestHit(ray);
        if (hit == null) {
            return scene.background;
        }
        Color local = phong(hit);
        double k = hit.sphere.material.reflectivity;
        if (k == 0.0 || depth >= MAX_DEPTH) {
            return local;
        }
        Vec3 reflectedDir = reflect(ray.direction, hit.normal);
        Ray reflectedRay = new Ray(
                hit.point.add(hit.normal.scale(Sphere.EPSILON)), reflectedDir);
        Color reflected = trace(reflectedRay, depth + 1);
        return local.scale(1.0 - k).add(reflected.scale(k));
    }

    /** Phong shading with a hard-shadow test per light (SPEC lessons 3-4). */
    public Color phong(Sphere.Hit hit) {
        Color colour = hit.sphere.material.color.scale(scene.ambientIntensity);
        Vec3 view = camera.position.sub(hit.point).normalize();
        for (Light light : scene.lights) {
            Vec3 toLight = light.position.sub(hit.point);
            double lightDistance = toLight.length();
            Vec3 l = toLight.normalize();
            double nDotL = hit.normal.dot(l);
            Ray shadowRay = new Ray(
                    hit.point.add(hit.normal.scale(Sphere.EPSILON)), l);
            boolean blocked = false;
            for (Sphere sphere : scene.spheres) {
                Sphere.Hit shadowHit = sphere.intersect(shadowRay);
                if (shadowHit != null && shadowHit.distance < lightDistance) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) {
                continue;
            }
            if (nDotL > 0.0) {
                colour = colour.add(
                        hit.sphere.material.color.multiply(light.color).scale(nDotL));
            }
            Vec3 r = reflect(l.scale(-1.0), hit.normal);
            double rDotV = Math.max(0.0, r.dot(view));
            if (rDotV > 0.0) {
                colour = colour.add(
                        light.color.scale(Math.pow(rDotV, hit.sphere.material.shininess)));
            }
        }
        return colour.clamp();
    }
}
