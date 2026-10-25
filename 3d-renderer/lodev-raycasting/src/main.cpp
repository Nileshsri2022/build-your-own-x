// Headless raycaster driver: renders ONE frame into a 640x480
// framebuffer and writes it as a binary PPM (P6) file.
//
// Display backend decision (docs left this open; SPEC section 7,
// DESIGN "decision required"): headless framebuffer only. No
// window mode is built or claimed here: this VM has no GUI
// toolkit/window available for a live window, and a window that
// was never run would be a false claim. The framebuffer is the
// same buffer a window backend would present, so every pixel
// behaviour in TEST_PLAN is checkable from the PPM / tests.
//
// Usage:
//   raycaster [--mode untextured|textured] [--output frame.ppm]
//             [--pos X Y] [--dir DX DY]
// Defaults: textured, frame.ppm, pos (12.5, 12.5), dir west (-1, 0).
// Plane is derived perpendicular to dir, length 0.66, as in SPEC.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "raycaster.h"

using namespace raycaster;

int main(int argc, char** argv) {
  bool textured = true;
  std::string out = "frame.ppm";
  Player p;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
      textured = std::strcmp(argv[++i], "untextured") != 0;
    } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
      out = argv[++i];
    } else if (std::strcmp(argv[i], "--pos") == 0 && i + 2 < argc) {
      p.posX = std::atof(argv[++i]);
      p.posY = std::atof(argv[++i]);
    } else if (std::strcmp(argv[i], "--dir") == 0 && i + 2 < argc) {
      p.dirX = std::atof(argv[++i]);
      p.dirY = std::atof(argv[++i]);
      // Perpendicular plane, same 0.66 length ratio as the default.
      p.planeX = -p.dirY * 0.66;
      p.planeY = p.dirX * 0.66;
    }
  }
  const World world = textured ? texturedMap() : untexturedMap();
  const Buffer buf = renderFrame(world, p, textured);
  FILE* f = std::fopen(out.c_str(), "wb");
  if (!f) {
    std::fprintf(stderr, "cannot open %s\n", out.c_str());
    return 1;
  }
  std::fprintf(f, "P6\n%d %d\n255\n", kScreenWidth, kScreenHeight);
  for (int px : buf) {
    const unsigned char rgb[3] = {static_cast<unsigned char>((px >> 16) & 0xFF),
                                  static_cast<unsigned char>((px >> 8) & 0xFF),
                                  static_cast<unsigned char>(px & 0xFF)};
    std::fwrite(rgb, 1, 3, f);
  }
  std::fclose(f);
  std::printf("wrote %s (%s, %dx%d)\n", out.c_str(),
              textured ? "textured" : "untextured", kScreenWidth, kScreenHeight);
  return 0;
}
