package com.renderer;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/** Thin ImageIO wrapper, so tests can write PNGs without touching Main. */
public final class PngWriter {
    private PngWriter() {
    }

    public static void write(BufferedImage image, String path) throws IOException {
        if (!ImageIO.write(image, "png", new File(path))) {
            throw new IOException("no PNG writer available for " + path);
        }
    }
}
