// CLI for the Scratchapixel raytracer: ./raytracer <output.ppm> [width height]
// Defaults: width = height = 200. Bad arguments -> usage on stderr, exit 1,
// no output file written. Write failures -> error on stderr, exit non-zero.
// Colour conversion per SPEC section 6: clamp to [0,1], round(channel*255).
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "raytracer.hpp"

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <output.ppm> [width height]\n";
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 4) {
        usage(argv[0]);
        return 1;
    }
    int width = 200, height = 200;
    if (argc == 4) {
        try {
            size_t used = 0;
            width = std::stoi(argv[2], &used);
            if (used != std::string(argv[2]).size()) throw std::invalid_argument("x");
            height = std::stoi(argv[3], &used);
            if (used != std::string(argv[3]).size()) throw std::invalid_argument("y");
        } catch (...) {
            usage(argv[0]);
            return 1;
        }
        if (width <= 0 || height <= 0) {
            usage(argv[0]);
            return 1;
        }
    }
    raytracer::Scene scene = raytracer::defaultScene();
    raytracer::Image img = raytracer::render(scene, width, height);
    if (!raytracer::writePpm(argv[1], img)) {
        std::cerr << "Error: cannot write " << argv[1] << '\n';
        return 1;
    }
    return 0;
}
