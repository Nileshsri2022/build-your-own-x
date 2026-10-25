// model.h — triangulated .obj mesh loader (own implementation).
// Vertices (v), texture coords (vt), vertex normals (vn), faces (f).
// A face must have exactly 3 vertices; anything else is an input error,
// reported, never silently triangulated (per DESIGN).
#pragma once
#include "geometry.h"
#include <string>
#include <vector>

struct Face {
    int v[3] = {-1, -1, -1};
    int vt[3] = {-1, -1, -1};
    int vn[3] = {-1, -1, -1};
    bool hasUV = false;
    bool hasNormal = false;
};

struct Model {
    std::vector<Vec3> verts;
    std::vector<Vec2> uvs;
    std::vector<Vec3> normals;
    std::vector<Face> faces;
};

bool loadObj(const std::string& path, Model& out, std::string& err);
