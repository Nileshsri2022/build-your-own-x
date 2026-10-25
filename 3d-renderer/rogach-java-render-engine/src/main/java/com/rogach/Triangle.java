package com.rogach;

import java.awt.Color;

/** Three vertices + a colour. Triangles are the only primitive. */
public final class Triangle {
    public final Vertex v1;
    public final Vertex v2;
    public final Vertex v3;
    public final Color color;

    public Triangle(Vertex v1, Vertex v2, Vertex v3, Color color) {
        this.v1 = v1;
        this.v2 = v2;
        this.v3 = v3;
        this.color = color;
    }
}
