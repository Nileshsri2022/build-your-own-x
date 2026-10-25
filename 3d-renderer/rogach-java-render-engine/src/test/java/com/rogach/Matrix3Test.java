package com.rogach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import org.junit.jupiter.api.Test;

class Matrix3Test {
    private static final double EPS = 1e-9;

    private static void assertVertex(Vertex expected, Vertex actual) {
        assertEquals(expected.x, actual.x, EPS);
        assertEquals(expected.y, actual.y, EPS);
        assertEquals(expected.z, actual.z, EPS);
    }

    @Test
    void identityLeavesVertexUnchanged() {
        assertVertex(new Vertex(1, 2, 3), Matrix3.identity().transform(new Vertex(1, 2, 3)));
    }

    @Test
    void xyRotation90MapsXToY() {
        assertVertex(new Vertex(0, 1, 0), Matrix3.rotationXY(90).transform(new Vertex(1, 0, 0)));
    }

    @Test
    void xzRotation90MapsXToMinusZ() {
        // Article's XZ convention as implemented (about y, x -> -z at 90 deg).
        assertVertex(new Vertex(0, 0, -1), Matrix3.rotationXZ(90).transform(new Vertex(1, 0, 0)));
    }

    @Test
    void multiplicationComposesLikeSequentialApplication() {
        Matrix3 heading = Matrix3.rotationXZ(30);
        Matrix3 pitch = Matrix3.rotationYZ(45);
        Matrix3 combined = heading.multiply(pitch);
        Vertex v = new Vertex(3, -2, 5);
        assertVertex(heading.transform(pitch.transform(v)), combined.transform(v));
    }

    @Test
    void zeroRotationIsIdentity() {
        Matrix3 combined = Matrix3.rotationXZ(0).multiply(Matrix3.rotationYZ(0));
        assertVertex(new Vertex(2, 3, 4), combined.transform(new Vertex(2, 3, 4)));
    }
}
