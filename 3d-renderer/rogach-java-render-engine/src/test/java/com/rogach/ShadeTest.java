package com.rogach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import java.awt.Color;
import java.awt.image.BufferedImage;
import java.util.List;
import org.junit.jupiter.api.Test;

class ShadeTest {
    @Test
    void shadeOneKeepsColourShadeZeroGivesBlack() {
        Color c = new Color(200, 100, 50);
        Color full = Shade.getShade(c, 1.0);
        assertEquals(200, full.getRed(), 3);
        assertEquals(100, full.getGreen(), 3);
        assertEquals(50, full.getBlue(), 3);
        Color black = Shade.getShade(c, 0.0);
        assertEquals(0, black.getRed());
        assertEquals(0, black.getGreen());
        assertEquals(0, black.getBlue());
    }

    @Test
    void facingSidewaysAnd45Degrees() {
        Vertex facing = Shade.normal(new Vertex(0, 0, 0), new Vertex(1, 0, 0), new Vertex(0, 1, 0));
        assertEquals(1.0, Shade.coefficient(facing), 1e-9);
        Vertex sideways = Shade.normal(new Vertex(0, 0, 0), new Vertex(0, 1, 0), new Vertex(0, 0, 1));
        assertEquals(0.0, Shade.coefficient(sideways), 1e-9);
        // 45 degrees: normal (sqrt(2)/2, 0, sqrt(2)/2), coefficient cos(45).
        double s = Math.sqrt(2) / 2;
        Vertex at45 = new Vertex(s, 0, s);
        assertEquals(Math.cos(Math.toRadians(45)), Shade.coefficient(at45), 1e-9);
        // Expected channel through the same 2.4 sRGB round-trip, tolerance +/-10.
        int expected = (int) Math.round(
                Math.pow(Math.pow(200 / 255.0, 2.4) * Math.cos(Math.toRadians(45)), 1.0 / 2.4) * 255);
        Color shaded = Shade.getShade(new Color(200, 0, 0), Shade.coefficient(at45));
        assertEquals(expected, shaded.getRed(), 10);
        assertTrue(shaded.getRed() > 0 && shaded.getRed() < 200);
    }

    @Test
    void shadingFollowsRotation() {
        // Normal (from unrotated vertices) has |nz| 0.8165; after a 90-degree
        // heading (XZ) rotation it has |nz| 0.4082, so the shade must change.
        // Pixel (240,220) in a 400x400 image lies inside the projected
        // triangle at both headings.
        Triangle t = new Triangle(new Vertex(0, 0, 0), new Vertex(100, 0, 50),
                new Vertex(0, 100, 50), Color.RED);
        BufferedImage at0 = Renderer.render(List.of(t), 0, 0, 400, 400);
        BufferedImage at90 = Renderer.render(List.of(t), 90, 0, 400, 400);
        int c0 = new Color(at0.getRGB(240, 220)).getRed();
        int c90 = new Color(at90.getRGB(240, 220)).getRed();
        assertTrue(c0 > 0 && c90 > 0);
        assertNotEquals(c0, c90);
        assertTrue(c0 > c90); // facing the light more at heading 0
    }
}
