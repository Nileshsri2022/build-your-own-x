// main.cpp — CLI: tinyrenderer <model.obj> [floor.obj] [output.tga]
// [options]. SPEC's argument order is kept; one divergence, recorded here:
// when exactly two positionals are given, a second argument ending in
// .tga is treated as the output file (SPEC says floor.obj), because
// otherwise a single model could never be rendered to a custom path.
// Flags (ours, for testability): --eye x,y,z, --light x,y,z,
// --diffuse file.tga, --normal file.tga (apply to the first model only),
// --no-shadow. Defaults: eye (0,0,3), centre (0,0,0), up (0,1,0),
// light (1,1,3), shadow on, output framebuffer.tga.
// Errors print to stderr and exit non-zero; no output file is written
// on any failure path (the TGA is written only after a full render).
#include "model.h"
#include "renderer.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static void usage() {
    std::fprintf(stderr,
        "usage: tinyrenderer <model.obj> [floor.obj] [output.tga] [options]\n"
        "  options: --eye x,y,z  --light x,y,z  --diffuse file.tga\n"
        "           --normal file.tga  --no-shadow\n");
}

static bool parseVec3(const char* s, Vec3& out) {
    return std::sscanf(s, "%lf,%lf,%lf", &out.x, &out.y, &out.z) == 3;
}

static bool endsWith(const std::string& s, const std::string& suf) {
    return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

int main(int argc, char** argv) {
    std::vector<std::string> pos;
    Vec3 eye(0, 0, 3), lightPos(1, 1, 3);
    std::string diffusePath, normalPath;
    bool useShadow = true;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--eye" && i + 1 < argc) { if (!parseVec3(argv[++i], eye)) { usage(); return 2; } }
        else if (a == "--light" && i + 1 < argc) { if (!parseVec3(argv[++i], lightPos)) { usage(); return 2; } }
        else if (a == "--diffuse" && i + 1 < argc) { diffusePath = argv[++i]; }
        else if (a == "--normal" && i + 1 < argc) { normalPath = argv[++i]; }
        else if (a == "--no-shadow") { useShadow = false; }
        else if (a.rfind("--", 0) == 0) { std::fprintf(stderr, "unknown option: %s\n", a.c_str()); usage(); return 2; }
        else pos.push_back(a);
    }
    if (pos.empty()) { usage(); return 2; }

    std::string modelPath = pos[0], floorPath, outputPath = "framebuffer.tga";
    if (pos.size() == 2) {
        if (endsWith(pos[1], ".tga")) outputPath = pos[1];
        else floorPath = pos[1];
    } else if (pos.size() == 3) {
        floorPath = pos[1]; outputPath = pos[2];
    } else if (pos.size() > 3) { usage(); return 2; }

    Model model, floor;
    std::string err;
    if (!loadObj(modelPath, model, err)) { std::fprintf(stderr, "error: %s\n", err.c_str()); return 1; }
    if (!floorPath.empty() && !loadObj(floorPath, floor, err)) { std::fprintf(stderr, "error: %s\n", err.c_str()); return 1; }

    TGAImage diffuse, normalMap;
    const TGAImage* diffusePtr = nullptr;
    const TGAImage* normalPtr = nullptr;
    if (!diffusePath.empty()) {
        if (!diffuse.read(diffusePath, err)) { std::fprintf(stderr, "error: %s\n", err.c_str()); return 1; }
        diffusePtr = &diffuse;
    }
    if (!normalPath.empty()) {
        if (!normalMap.read(normalPath, err)) { std::fprintf(stderr, "error: %s\n", err.c_str()); return 1; }
        normalPtr = &normalMap;
    }

    std::vector<SceneModel> models;
    models.push_back({&model, diffusePtr, normalPtr});
    if (!floorPath.empty()) models.push_back({&floor, nullptr, nullptr});

    Renderer renderer;
    renderer.render(models, eye, Vec3(0, 0, 0), Vec3(0, 1, 0), Light{lightPos}, useShadow);
    if (!renderer.framebuffer.write(outputPath, err)) { std::fprintf(stderr, "error: %s\n", err.c_str()); return 1; }
    return 0;
}
