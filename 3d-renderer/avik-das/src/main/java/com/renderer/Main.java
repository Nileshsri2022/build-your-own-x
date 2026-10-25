package com.renderer;

import java.awt.image.BufferedImage;
import java.io.IOException;

/**
 * CLI: Main &lt;output.png&gt; [width height samples].
 * Defaults: 400 400, samples 1. Bad arguments print usage to stderr,
 * exit 1, and leave no file behind.
 */
public final class Main {
    private Main() {
    }

    public static void main(String[] args) {
        if (args.length != 1 && args.length != 4) {
            usage();
            System.exit(1);
        }
        String output = args[0];
        if (!output.endsWith(".png")) {
            System.err.println("error: output must end in .png: " + output);
            usage();
            System.exit(1);
        }
        int width = 400;
        int height = 400;
        int samples = 1;
        if (args.length == 4) {
            try {
                width = Integer.parseInt(args[1]);
                height = Integer.parseInt(args[2]);
                samples = Integer.parseInt(args[3]);
            } catch (NumberFormatException e) {
                System.err.println("error: width, height and samples must be integers");
                usage();
                System.exit(1);
            }
            if (width <= 0 || height <= 0 || samples <= 0) {
                System.err.println("error: width, height and samples must be positive");
                usage();
                System.exit(1);
            }
        }
        Scene scene = Scene.defaultScene();
        Camera camera = Scene.defaultCamera();
        BufferedImage image = new Renderer(scene, camera).render(width, height, samples);
        try {
            PngWriter.write(image, output);
        } catch (IOException e) {
            System.err.println("error: failed to write " + output + ": " + e.getMessage());
            System.exit(1);
        }
    }

    private static void usage() {
        System.err.println("usage: Main <output.png> [width height samples]");
    }
}
