package com.rogach;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 * Our addition, not the article's: writes the same BufferedImage the
 * window draws to a .png path, so tests can check pixels headlessly.
 * No gamma correction or tone mapping beyond the shading step.
 */
public final class PngWriter {
    private PngWriter() {
    }

    public static void write(BufferedImage image, String path) throws IOException {
        ImageIO.write(image, "png", new File(path));
    }
}
