package com.rogach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import java.awt.Color;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import org.junit.jupiter.api.Test;

class InflateTest {
    @Test
    void countsQuadruplePerRound() {
        List<Triangle> tetra = Main.tetrahedron();
        assertEquals(4, tetra.size());
        assertEquals(16, Inflate.inflate(tetra, 100, 1).size());
        assertEquals(64, Inflate.inflate(tetra, 100, 2).size());
    }

    @Test
    void allVerticesOnSphereAndColoursInherited() {
        List<Triangle> inflated = Inflate.inflate(Main.tetrahedron(), 100, 1);
        Set<Color> colours = new HashSet<>();
        for (Triangle t : inflated) {
            colours.add(t.color);
            assertEquals(100, t.v1.length(), 1e-6);
            assertEquals(100, t.v2.length(), 1e-6);
            assertEquals(100, t.v3.length(), 1e-6);
        }
        assertEquals(Set.of(Color.WHITE, Color.RED, Color.GREEN, Color.BLUE), colours);
    }

    @Test
    void zeroRoundsIsIdentity() {
        List<Triangle> tetra = Main.tetrahedron();
        List<Triangle> same = Inflate.inflate(tetra, 100, 0);
        assertEquals(tetra.size(), same.size());
        for (int i = 0; i < tetra.size(); i++) {
            assertEquals(tetra.get(i).v1.x, same.get(i).v1.x, 0);
            assertEquals(tetra.get(i).v1.y, same.get(i).v1.y, 0);
            assertEquals(tetra.get(i).v1.z, same.get(i).v1.z, 0);
            assertEquals(tetra.get(i).color, same.get(i).color);
        }
    }
}
