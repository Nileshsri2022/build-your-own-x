package com.rogach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import java.awt.Color;
import java.awt.image.BufferedImage;
import java.util.List;
import org.junit.jupiter.api.Test;

class RasterTest {
    // Facing triangle (normal (0,0,1), shade 1, colour unchanged):
    // vertices (-20,-20),(20,-20),(0,20) in object space.
    private static Triangle facingTriangle(Color color, double z) {
        return new Triangle(new Vertex(-20, -20, z), new Vertex(20, -20, z),
                new Vertex(0, 20, z), color);
    }

    private static boolean isColoured(BufferedImage img, int x, int y) {
        return (img.getRGB(x, y) >>> 24) != 0;
    }

    @Test
    void centroidColouredOutsideNot() {
        BufferedImage img = Renderer.render(
                List.of(facingTriangle(Color.RED, 0)), 0, 0, 100, 100);
        // Centroid of (-20,-20),(20,-20),(0,20) is (0,-6.67) -> screen (50,43).
        assertTrue(isColoured(img, 50, 43));
        assertTrue(!isColoured(img, 5, 5));
        assertEquals(Color.RED.getRGB(), img.getRGB(50, 43));
    }

    @Test
    void wireframeHasOutlineButEmptyInterior() {
        BufferedImage img = Renderer.renderWireframe(
                List.of(facingTriangle(Color.RED, 0)), 0, 0, 100, 100);
        assertTrue(isColoured(img, 50, 30)); // top edge, y = 50-20
        assertTrue(!isColoured(img, 50, 43)); // centroid: interior, not an edge
    }

    @Test
    void trianglePartlyOutsideImageClampsWithoutThrowing() {
        Triangle big = new Triangle(new Vertex(-100, -100, 0), new Vertex(100, -100, 0),
                new Vertex(0, 100, 0), Color.BLUE);
        BufferedImage img = Renderer.render(List.of(big), 0, 0, 100, 100);
        assertTrue(isColoured(img, 50, 50));
    }

    @Test
    void degenerateTriangleRendersNothing() {
        Triangle degenerate = new Triangle(new Vertex(0, 0, 0), new Vertex(10, 0, 0),
                new Vertex(20, 0, 0), Color.RED);
        BufferedImage img = Renderer.render(List.of(degenerate), 0, 0, 100, 100);
        assertTrue(!isColoured(img, 50, 50));
        assertTrue(!isColoured(img, 60, 50));
    }

    @Test
    void nearerTriangleWinsRegardlessOfListOrder() {
        Triangle far = facingTriangle(Color.BLUE, -10);
        Triangle near = facingTriangle(Color.RED, 10);
        BufferedImage first = Renderer.render(List.of(far, near), 0, 0, 100, 100);
        assertEquals(Color.RED.getRGB(), first.getRGB(50, 43));
        // Depths swapped: the other colour must now win.
        Triangle farRed = facingTriangle(Color.RED, -10);
        Triangle nearBlue = facingTriangle(Color.BLUE, 10);
        BufferedImage second = Renderer.render(List.of(farRed, nearBlue), 0, 0, 100, 100);
        assertEquals(Color.BLUE.getRGB(), second.getRGB(50, 43));
        assertNotEquals(first.getRGB(50, 43), second.getRGB(50, 43));
    }

    @Test
    void fullyBehindTriangleContributesNothing() {
        Triangle front = facingTriangle(Color.RED, 10);
        Triangle behind = new Triangle(new Vertex(-5, -5, -10), new Vertex(5, -5, -10),
                new Vertex(0, 5, -10), Color.BLUE);
        BufferedImage img = Renderer.render(List.of(front, behind), 0, 0, 100, 100);
        // Behind triangle's centroid projects to screen (50,48), inside the front triangle.
        assertEquals(Color.RED.getRGB(), img.getRGB(50, 48));
    }
}
