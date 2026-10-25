package com.rogach;

import java.util.ArrayList;
import java.util.List;

/**
 * Sphere approximation (bonus): each round splits every triangle into
 * four by joining edge midpoints, then pushes every vertex out to the
 * sphere radius (divide by its length, times radius). Colours are
 * inherited per original triangle. One round quadruples the count.
 * Zero rounds returns the same triangles, colours and vertices.
 * A vertex at the origin has no direction to push, so it is left as-is.
 */
public final class Inflate {
    private Inflate() {
    }

    public static List<Triangle> inflate(List<Triangle> triangles, double radius, int rounds) {
        List<Triangle> result = new ArrayList<>(triangles);
        for (int r = 0; r < rounds; r++) {
            List<Triangle> next = new ArrayList<>();
            for (Triangle t : result) {
                Vertex m1 = midpoint(t.v1, t.v2);
                Vertex m2 = midpoint(t.v2, t.v3);
                Vertex m3 = midpoint(t.v3, t.v1);
                next.add(new Triangle(t.v1, m1, m3, t.color));
                next.add(new Triangle(t.v2, m1, m2, t.color));
                next.add(new Triangle(t.v3, m2, m3, t.color));
                next.add(new Triangle(m1, m2, m3, t.color));
            }
            result = new ArrayList<>();
            for (Triangle t : next) {
                result.add(new Triangle(onSphere(t.v1, radius),
                        onSphere(t.v2, radius), onSphere(t.v3, radius), t.color));
            }
        }
        return result;
    }

    private static Vertex midpoint(Vertex a, Vertex b) {
        return new Vertex((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
    }

    private static Vertex onSphere(Vertex v, double radius) {
        double len = v.length();
        if (len == 0) {
            return v;
        }
        return v.scaled(radius / len);
    }
}
