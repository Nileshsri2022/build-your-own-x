package com.renderer;

import java.awt.image.BufferedImage;
import java.io.File;
import javax.imageio.ImageIO;

/**
 * Integration pixel check: reads a PNG, asserts dimensions, centre pixel
 * predominantly red, corner pixel background black. Usage: PixelCheck <png> <w> <h>.
 */
public final class PixelCheck {
    public static void main(String[] args) throws Exception {
        BufferedImage img = ImageIO.read(new File(args[0]));
        if (img == null) {
            System.out.println("FAIL: not a readable PNG: " + args[0]);
            System.exit(1);
        }
        int w = Integer.parseInt(args[1]);
        int h = Integer.parseInt(args[2]);
        if (img.getWidth() != w || img.getHeight() != h) {
            System.out.println("FAIL: dimensions " + img.getWidth() + "x" + img.getHeight()
                    + ", expected " + w + "x" + h);
            System.exit(1);
        }
        int centre = img.getRGB(w / 2, h / 2);
        int r = (centre >> 16) & 0xFF;
        int g = (centre >> 8) & 0xFF;
        int b = centre & 0xFF;
        System.out.println("centre pixel: " + r + "," + g + "," + b);
        if (!(r > g + 50 && r > b + 50)) {
            System.out.println("FAIL: centre pixel not predominantly red");
            System.exit(1);
        }
        if ((img.getRGB(0, 0) & 0xFFFFFF) != 0) {
            System.out.println("FAIL: corner pixel not background black");
            System.exit(1);
        }
        System.out.println("PIXELCHECK PASS");
    }
}
