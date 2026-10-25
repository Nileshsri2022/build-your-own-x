package com.renderer;

import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;
import javax.imageio.ImageIO;

/**
 * Plain-Java test harness (no JUnit jar is available in this VM, and Maven
 * Central is unreachable from this VM's Java, so JUnit cannot be downloaded
 * here). It covers the same cases as TEST_PLAN.md's Vec3Test, SphereTest,
 * ShadingTest and RendererTest; pom.xml still declares JUnit 5 as the
 * canonical test dependency for a normal machine.
 * Run: java -cp <classes> com.renderer.RendererChecks
 */
public final class RendererChecks {
    private static int passed = 0;
    private static int failed = 0;

    private static void check(String name, boolean cond) {
        if (cond) {
            passed++;
            System.out.println("PASS " + name);
        } else {
            failed++;
            System.out.println("FAIL " + name);
        }
    }

    private static void approx(String name, double actual, double expected, double tol) {
        check(name + " (actual=" + actual + ", expected=" + expected + ")", Math.abs(actual - expected) <= tol);
    }

    private static int[] rgb(BufferedImage img, int x, int y) {
        int p = img.getRGB(x, y);
        return new int[]{(p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF};
    }

    private static void rgbApprox(String name, int[] actual, int[] expected, int tol) {
        boolean ok = true;
        for (int i = 0; i < 3; i++) {
            if (Math.abs(actual[i] - expected[i]) > tol) {
                ok = false;
            }
        }
        check(name + " (actual=" + actual[0] + "," + actual[1] + "," + actual[2]
                + ", expected=" + expected[0] + "," + expected[1] + "," + expected[2] + ")", ok);
    }

    public static void main(String[] args) throws Exception {
        // ---- Vec3 ----
        Vec3 a = new Vec3(1, 2, 3);
        Vec3 b = new Vec3(4, -1, 0);
        check("vec3 add", a.add(b).x == 5 && a.add(b).y == 1 && a.add(b).z == 3);
        check("vec3 sub", a.sub(b).x == -3 && a.sub(b).y == 3 && a.sub(b).z == 3);
        check("vec3 scale", a.scale(2).x == 2 && a.scale(2).y == 4 && a.scale(2).z == 6);
        approx("vec3 dot perpendicular", new Vec3(1, 0, 0).dot(new Vec3(0, 1, 0)), 0, 1e-12);
        approx("vec3 dot parallel", new Vec3(2, 0, 0).dot(new Vec3(3, 0, 0)), 6, 1e-12);
        Vec3 cr = new Vec3(1, 0, 0).cross(new Vec3(0, 1, 0));
        check("vec3 cross right-hand rule", cr.x == 0 && cr.y == 0 && cr.z == 1);
        approx("vec3 length", new Vec3(3, 4, 0).length(), 5, 1e-12);
        approx("vec3 normalize unit length", new Vec3(3, 4, 0).normalize().length(), 1, 1e-12);
        boolean zeroThrows = false;
        try {
            new Vec3(0, 0, 0).normalize();
        } catch (IllegalArgumentException e) {
            zeroThrows = true;
        }
        check("vec3 normalize zero vector throws (not NaN)", zeroThrows);
        check("vec3 lerp 0", Vec3.lerp(a, b, 0).x == 1);
        check("vec3 lerp 0.5", Vec3.lerp(a, b, 0.5).x == 2.5 && Vec3.lerp(a, b, 0.5).y == 0.5);
        check("vec3 lerp 1", Vec3.lerp(a, b, 1).x == 4);

        // ---- Camera ----
        Camera cam = Scene.defaultCamera();
        Vec3 centreDir = cam.rayForPixel(20, 20, 41, 41).direction.normalize();
        approx("camera centre ray forward x", centreDir.x, 0, 1e-9);
        approx("camera centre ray forward z", centreDir.z, 1, 1e-9);
        Vec3 cornerDir = cam.rayForPixel(0, 0, 2, 2).direction.normalize();
        Vec3 expectedCorner = new Vec3(-0.5, 0.5, 2).normalize();
        approx("camera corner ray points to corner", cornerDir.dot(expectedCorner), 1, 1e-9);

        // ---- Sphere ----
        Material matteRed = new Material(new Color(1, 0, 0), 50, 0.0);
        Sphere sphere = new Sphere(new Vec3(0, 0, 5), 1.0, matteRed);
        Ray straight = new Ray(new Vec3(0, 0, 0), new Vec3(0, 0, 1));
        Sphere.Hit hit = sphere.intersect(straight);
        approx("sphere centre hit distance", hit == null ? -1 : hit.distance, 4, 1e-9);
        check("sphere miss to the side",
                sphere.intersect(new Ray(new Vec3(2, 0, 0), new Vec3(0, 0, 1))) == null);
        Sphere.Hit tangent = sphere.intersect(new Ray(new Vec3(1, 0, 0), new Vec3(0, 0, 1)));
        check("sphere tangent has one hit", tangent != null && Math.abs(tangent.distance - 5) < 1e-9);
        Sphere.Hit inside = sphere.intersect(new Ray(new Vec3(0, 0, 5), new Vec3(0, 0, 1)));
        check("sphere ray from inside exits at positive t", inside != null && inside.distance > 0);
        check("sphere ray pointing away has no hit",
                sphere.intersect(new Ray(new Vec3(0, 0, 0), new Vec3(0, 0, -1))) == null);
        Sphere near = new Sphere(new Vec3(0, 0, 5), 1.0, matteRed);
        Sphere far = new Sphere(new Vec3(0, 0, 8), 1.0, matteRed);
        Scene two = new Scene(List.of(far, near), List.of(), Color.BLACK, 0.1);
        Sphere.Hit nearest = two.nearestHit(straight);
        check("nearest hit wins and normal points outward",
                nearest != null && nearest.sphere == near && nearest.normal.z < 0);

        // ---- Shading (hand-computed, 41x41 so centre pixel is the exact centre ray) ----
        // Default scene centre: ambient 0.1 + diffuse N.L=0.4924, specular ~0 -> (151,0,0).
        Renderer def = new Renderer(Scene.defaultScene(), Scene.defaultCamera());
        rgbApprox("shading default centre", rgb(def.render(41, 41, 1), 20, 20), new int[]{151, 0, 0}, 3);
        // Light behind the surface: ambient only -> (26,0,0).
        Scene behind = new Scene(
                List.of(new Sphere(new Vec3(0, 0, 5), 1.0, matteRed)),
                List.of(new Light(new Vec3(0, 0, 10), Color.WHITE)), Color.BLACK, 0.1);
        rgbApprox("shading light behind = ambient only",
                rgb(new Renderer(behind, Scene.defaultCamera()).render(41, 41, 1), 20, 20),
                new int[]{26, 0, 0}, 3);
        // Light at the camera: diffuse 1 + specular 1 -> clamped white.
        Scene atCam = new Scene(
                List.of(new Sphere(new Vec3(0, 0, 5), 1.0, matteRed)),
                List.of(new Light(new Vec3(0, 0, 0), Color.WHITE)), Color.BLACK, 0.1);
        rgbApprox("shading specular near 1 at camera light",
                rgb(new Renderer(atCam, Scene.defaultCamera()).render(41, 41, 1), 20, 20),
                new int[]{255, 255, 255}, 3);
        // Shadow: blocker between hit (0,0,4) and light (-5,5,0) removes diffuse/specular.
        Vec3 blockerCentre = new Vec3(-1.231, 1.231, 3.015);
        Scene shadowed = new Scene(
                List.of(new Sphere(new Vec3(0, 0, 5), 1.0, matteRed),
                        new Sphere(blockerCentre, 0.5, matteRed)),
                List.of(new Light(new Vec3(-5, 5, 0), Color.WHITE)), Color.BLACK, 0.1);
        rgbApprox("shading shadow blocked = ambient only",
                rgb(new Renderer(shadowed, Scene.defaultCamera()).render(41, 41, 1), 20, 20),
                new int[]{26, 0, 0}, 3);
        // Reflection: k=1 sphere reflects a red sphere behind the camera.
        Scene reflective = new Scene(
                List.of(new Sphere(new Vec3(0, 0, 5), 1.0,
                                new Material(Color.WHITE, 50, 1.0)),
                        new Sphere(new Vec3(0, 0, -5), 1.0, matteRed)),
                List.of(new Light(new Vec3(-5, 5, 0), Color.WHITE)), Color.BLACK, 0.1);
        int[] refl = rgb(new Renderer(reflective, Scene.defaultCamera()).render(41, 41, 1), 20, 20);
        check("reflection k=1 shows reflected colour, not own colour (red dominant)",
                refl[0] > 100 && refl[1] < 10 && refl[2] < 10);

        // ---- Renderer ----
        Scene empty = new Scene(List.of(), List.of(), Color.BLACK, 0.1);
        BufferedImage emptyImg = new Renderer(empty, Scene.defaultCamera()).render(10, 10, 1);
        // getRGB returns ARGB (alpha 0xFF), so black is 0xFF000000, not 0.
        boolean allBlack = true;
        for (int y = 0; y < 10 && allBlack; y++) {
            for (int x = 0; x < 10; x++) {
                if ((emptyImg.getRGB(x, y) & 0xFFFFFF) != 0) {
                    allBlack = false;
                }
            }
        }
        check("renderer empty scene all black", allBlack);
        BufferedImage small = def.render(40, 40, 1);
        check("renderer default dimensions", small.getWidth() == 40 && small.getHeight() == 40);
        int[] centre40 = rgb(small, 20, 20);
        check("renderer default centre predominantly red",
                centre40[0] > centre40[1] + 50 && centre40[0] > centre40[2] + 50);
        check("renderer default corner black", (small.getRGB(0, 0) & 0xFFFFFF) == 0);
        BufferedImage aa = def.render(40, 40, 4);
        int[] centreAA = rgb(aa, 20, 20);
        int diffCount = 0;
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                int[] p1 = rgb(small, x, y);
                int[] p4 = rgb(aa, x, y);
                if (Math.abs(p1[0] - p4[0]) > 3 || Math.abs(p1[1] - p4[1]) > 3
                        || Math.abs(p1[2] - p4[2]) > 3) {
                    diffCount++;
                }
            }
        }
        check("AA S=1 vs S=4: flat centre unchanged",
                Math.abs(centre40[0] - centreAA[0]) <= 3
                        && Math.abs(centre40[1] - centreAA[1]) <= 3
                        && Math.abs(centre40[2] - centreAA[2]) <= 3);
        check("AA S=1 vs S=4: only a minority of pixels differ (edges)",
                diffCount < 40 * 40 / 4);
        ByteArrayOutputStream b1 = new ByteArrayOutputStream();
        ByteArrayOutputStream b2 = new ByteArrayOutputStream();
        ImageIO.write(def.render(20, 20, 4), "png", b1);
        ImageIO.write(def.render(20, 20, 4), "png", b2);
        check("determinism: same seed+samples -> byte-identical PNG",
                java.util.Arrays.equals(b1.toByteArray(), b2.toByteArray()));

        System.out.println("RESULT passed=" + passed + " failed=" + failed);
        if (failed > 0) {
            System.exit(1);
        }
    }
}
