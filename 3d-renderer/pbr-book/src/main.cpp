// CLI for the scoped PBR path tracer.
// Usage: pbrt <output.ppm> [width height samples seed]
// Defaults: 100 100 16 42. No arguments -> usage on stderr, exit 2.
// Output: PPM (P3), our proposal — NOT a format claimed from the book.
// Default scene (ours, not the book's — SPEC §4 deliberately does not
// invent a book scene): ground + diffuse red sphere at the origin,
// a mirror sphere left, a glass sphere right, one point light.
#include <iostream>
#include <string>
#include <cstdlib>
#include "geometry.h"
#include "camera.h"
#include "film.h"
#include "primitives.h"
#include "integrator.h"

using namespace pbr;

static std::vector<Primitive> defaultScene() {
    std::vector<Primitive> prims;
    auto add = [&](std::shared_ptr<Shape> s, std::shared_ptr<BSDF> b) {
        prims.push_back(Primitive{std::move(s), std::move(b)});
    };
    add(std::make_shared<Sphere>(Vec3{0, -1000, 0}, 1000),
        std::make_shared<DiffuseBSDF>(Spectrum{0.5, 0.5, 0.5}));
    add(std::make_shared<Sphere>(Vec3{0, 0, 0}, 1),
        std::make_shared<DiffuseBSDF>(Spectrum{0.8, 0.1, 0.1}));
    add(std::make_shared<Sphere>(Vec3{-2.2, 0, 0}, 1),
        std::make_shared<MirrorBSDF>(Spectrum{0.9, 0.9, 0.9}));
    add(std::make_shared<Sphere>(Vec3{2.2, 0, 0}, 1),
        std::make_shared<DielectricBSDF>(1.5));
    return prims;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: pbrt <output.ppm> [width height samples seed]\n";
        return 2;
    }
    std::string outPath = argv[1];
    int width = argc > 2 ? std::atoi(argv[2]) : 100;
    int height = argc > 3 ? std::atoi(argv[3]) : 100;
    int samples = argc > 4 ? std::atoi(argv[4]) : 16;
    uint64_t seed = argc > 5 ? std::strtoull(argv[5], nullptr, 10) : 42;
    if (width <= 0 || height <= 0 || samples <= 0) {
        std::cerr << "error: width, height and samples must be positive\n";
        return 2;
    }
    BVH bvh(defaultScene());
    PathTracer tracer(bvh, {PointLight{Vec3{-4, 6, 4}, Spectrum{80, 80, 80}}});
    ProjectiveCamera cam(Vec3{0, 1, 6}, Vec3{0, 0, 0}, Vec3{0, 1, 0}, 40,
                         (double)width / height);
    Film film(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Sampler s(seed * 1000003ULL + (uint64_t)(y * width + x));
            Spectrum acc{};
            for (int i = 0; i < samples; ++i) {
                double u = (x + s.uniform()) / width;
                double v = 1 - (y + s.uniform()) / height;
                acc += tracer.Li(cam.generateRay(u, v), s);
            }
            film.addSample(x, y, acc / (double)samples);
        }
    }
    if (!film.writePPM(outPath)) {
        std::cerr << "error: cannot write output file: " << outPath << "\n";
        return 1;
    }
    return 0;
}
