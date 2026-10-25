#include "model.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static bool parseIndex(const char* tok, int count, int& out, std::string& err) {
    if (!tok || !*tok) return true;  // empty field = absent
    int v = std::atoi(tok);
    if (v == 0 || v > count || v < -count) { err = "obj index out of range"; return false; }
    out = v > 0 ? v - 1 : count + v;
    return true;
}

bool loadObj(const std::string& path, Model& out, std::string& err) {
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) { err = "cannot open model file: " + path; return false; }
    char line[1024];
    int lineno = 0;
    while (std::fgets(line, sizeof(line), f)) {
        lineno++;
        if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            if (std::sscanf(line + 2, "%lf %lf %lf", &x, &y, &z) != 3) { err = "bad vertex at line " + std::to_string(lineno); std::fclose(f); return false; }
            out.verts.push_back({x, y, z});
        } else if (line[0] == 'v' && line[1] == 't') {
            double u, v;
            if (std::sscanf(line + 3, "%lf %lf", &u, &v) != 2) { err = "bad texcoord at line " + std::to_string(lineno); std::fclose(f); return false; }
            out.uvs.push_back({u, v});
        } else if (line[0] == 'v' && line[1] == 'n') {
            double x, y, z;
            if (std::sscanf(line + 3, "%lf %lf %lf", &x, &y, &z) != 3) { err = "bad normal at line " + std::to_string(lineno); std::fclose(f); return false; }
            out.normals.push_back({x, y, z});
        } else if (line[0] == 'f' && line[1] == ' ') {
            // Count vertex tokens first: must be exactly 3.
            char copy[1024];
            std::strncpy(copy, line + 2, sizeof(copy) - 1); copy[sizeof(copy) - 1] = 0;
            int ntok = 0;
            for (char* t = std::strtok(copy, " \t\r\n"); t; t = std::strtok(nullptr, " \t\r\n")) ntok++;
            if (ntok != 3) { err = "non-triangulated face at line " + std::to_string(lineno) + " (faces must have exactly 3 vertices)"; std::fclose(f); return false; }
            Face face;
            std::strncpy(copy, line + 2, sizeof(copy) - 1); copy[sizeof(copy) - 1] = 0;
            int i = 0;
            for (char* t = std::strtok(copy, " \t\r\n"); t && i < 3; t = std::strtok(nullptr, " \t\r\n"), i++) {
                // Split v/vt/vn, keeping empty middle field (v//vn).
                char* slash1 = std::strchr(t, '/');
                char* vTok = t;
                char* vtTok = nullptr;
                char* vnTok = nullptr;
                if (slash1) {
                    *slash1 = 0;
                    vtTok = slash1 + 1;
                    char* slash2 = std::strchr(vtTok, '/');
                    if (slash2) { *slash2 = 0; vnTok = slash2 + 1; }
                }
                if (!parseIndex(vTok, (int)out.verts.size(), face.v[i], err) ||
                    !parseIndex(vtTok, (int)out.uvs.size(), face.vt[i], err) ||
                    !parseIndex(vnTok, (int)out.normals.size(), face.vn[i], err)) {
                    std::fclose(f); return false;
                }
                if (face.vt[i] >= 0) face.hasUV = true;
                if (face.vn[i] >= 0) face.hasNormal = true;
            }
            out.faces.push_back(face);
        }
        // Other lines (comments, mtllib, o, g, s, ...) are ignored.
    }
    std::fclose(f);
    if (out.verts.empty()) { err = "model has no vertices: " + path; return false; }
    return true;
}
