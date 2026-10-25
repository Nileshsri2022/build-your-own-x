package com.rogach;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.util.List;
import javax.swing.SwingUtilities;

/**
 * Entry point. No arguments: opens the Swing window (DemoViewer).
 * With arguments: Main out.png [width height [inflateRounds]]
 * renders the tetrahedron at heading 180 / pitch 0 and saves it as
 * a PNG (our addition, for headless tests) instead of opening a
 * window. Bad sizes exit non-zero with an error on stderr and
 * create no file.
 */
public final class Main {
    private Main() {
    }

    /** Tetrahedron centred at the origin, 4 faces: white, red, green, blue. */
    public static List<Triangle> tetrahedron() {
        Vertex a = new Vertex(100, 100, 100);
        Vertex b = new Vertex(100, -100, -100);
        Vertex c = new Vertex(-100, 100, -100);
        Vertex d = new Vertex(-100, -100, 100);
        return List.of(
                new Triangle(a, b, c, Color.WHITE),
                new Triangle(a, b, d, Color.RED),
                new Triangle(a, c, d, Color.GREEN),
                new Triangle(b, c, d, Color.BLUE));
    }

    public static void main(String[] args) throws Exception {
        if (args.length == 0) {
            List<Triangle> tetra = tetrahedron();
            SwingUtilities.invokeLater(() -> new DemoViewer(tetra).show());
            return;
        }
        String path = args[0];
        int width = 400;
        int height = 400;
        int inflateRounds = 0;
        try {
            if (args.length >= 3) {
                width = Integer.parseInt(args[1]);
                height = Integer.parseInt(args[2]);
            }
            if (args.length >= 4) {
                inflateRounds = Integer.parseInt(args[3]);
            }
        } catch (NumberFormatException e) {
            System.err.println("Usage: Main [out.png [width height [inflateRounds]]]");
            System.exit(1);
        }
        if (width <= 0 || height <= 0 || inflateRounds < 0) {
            System.err.println("Usage: Main [out.png [width height [inflateRounds]]]: "
                    + "width and height must be positive, inflateRounds must be >= 0");
            System.exit(1);
        }
        List<Triangle> triangles = tetrahedron();
        if (inflateRounds > 0) {
            // Sphere radius = the tetrahedron vertex length, so the
            // inflated sphere fills most of the image.
            double radius = tetrahedron().get(0).v1.length();
            triangles = Inflate.inflate(triangles, radius, inflateRounds);
        }
        BufferedImage image = Renderer.render(triangles, 180, 0, width, height);
        PngWriter.write(image, path);
    }
}
