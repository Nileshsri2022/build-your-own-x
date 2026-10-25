package com.rogach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import java.awt.Color;
import java.awt.image.BufferedImage;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import org.junit.jupiter.api.Test;

class RendererTest {
    @Test
    void emptyListRendersBlankImageOfRequestedSize() {
        BufferedImage img = Renderer.render(List.of(), 0, 0, 200, 200);
        assertEquals(200, img.getWidth());
        assertEquals(200, img.getHeight());
        assertEquals(0, img.getRGB(0, 0));
        assertEquals(0, img.getRGB(100, 100));
    }

    @Test
    void tetrahedronIsNonBlankCentreColouredAndMultiColoured() {
        BufferedImage img = Renderer.render(Main.tetrahedron(), 180, 0, 200, 200);
        assertTrue((img.getRGB(100, 100) >>> 24) != 0); // centre coloured
        Set<Integer> colours = new HashSet<>();
        boolean nonBlank = false;
        for (int y = 0; y < 200; y++) {
            for (int x = 0; x < 200; x++) {
                int rgb = img.getRGB(x, y);
                if ((rgb >>> 24) != 0) {
                    nonBlank = true;
                    colours.add(rgb);
                }
            }
        }
        assertTrue(nonBlank);
        // At least two distinct (shaded) face colours in distinct regions.
        assertTrue(colours.size() >= 2);
    }

    @Test
    void rendersAreDeterministic() {
        BufferedImage a = Renderer.render(Main.tetrahedron(), 180, 0, 200, 200);
        BufferedImage b = Renderer.render(Main.tetrahedron(), 180, 0, 200, 200);
        for (int y = 0; y < 200; y++) {
            for (int x = 0; x < 200; x++) {
                assertEquals(a.getRGB(x, y), b.getRGB(x, y));
            }
        }
    }

    @Test
    void flatShadingIsPerTriangleNotPerPixel() {
        Triangle t = new Triangle(new Vertex(-20, -20, 0), new Vertex(20, -20, 0),
                new Vertex(0, 20, 0), Color.RED);
        BufferedImage img = Renderer.render(List.of(t), 0, 0, 100, 100);
        Set<Integer> colours = new HashSet<>();
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                int rgb = img.getRGB(x, y);
                if ((rgb >>> 24) != 0) {
                    colours.add(rgb);
                }
            }
        }
        assertEquals(1, colours.size()); // one shade for the whole face
    }

    @Test
    void nonPositiveSizeIsRejected() {
        try {
            Renderer.render(List.of(), 0, 0, 0, 100);
            assertTrue(false, "expected IllegalArgumentException");
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }
}
