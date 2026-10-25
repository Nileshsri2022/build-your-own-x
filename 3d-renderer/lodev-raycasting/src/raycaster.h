// Raycaster core: Lodev-style DDA raycasting over a 24x24 grid.
//
// This is a raycaster (semi-3D first-person walker), NOT a true 3D
// renderer: flat grid world, equal-height orthogonal walls, no
// spheres/triangles/meshes, no Phong, no shadows, no reflections,
// no refraction, no floors/ceilings, no sprites, no stairs.
//
// Implementation written from the SPEC/DESIGN concepts, not copied
// from the tutorial. Recorded choices (see docs/SPEC.md, DESIGN.md):
//  - Zero ray components use IEEE-754 infinity (C++ double division
//    by zero), not the generic 1e30 guard. Infinity loses every
//    sideDist comparison, so that axis is never stepped on, which
//    is exactly the behaviour the DDA needs.
//  - Deltas use the SPEC simplified form: deltaX = |1/rayDirX|,
//    deltaY = |1/rayDirY| (ray length taken as 1 in one component),
//    NOT the Euclidean sqrt form. Only the ratio matters for the
//    stepping decision (it is identical either way), but the
//    simplified form is what makes perpWallDist = (last side
//    distance minus one delta) equal the PERPENDICULAR distance.
//    Using Euclidean deltas with that formula would give the
//    along-ray distance and reintroduce fisheye; this was caught
//    by the no-fisheye test during development and fixed here.
//  - Movement collision is checked per axis (x move, then y move),
//    as in DESIGN: sliding along a wall at a corner is possible,
//    walking diagonally into a corner is not a single rejected move.
//  - Texture mapping is affine (linear texel stepping), correct
//    for vertical walls, NOT perspective-correct in general.
//  - DDA loop is capped (kMaxSteps) so an open map cannot hang;
//    shipped maps are border-walled, so the cap is never reached.
//  - Y-side darkening halves each packed RGB channel:
//    (colour >> 1) & 0x7F7F7F. The mask 8355711 = 0x7F7F7F clears
//    the top bit of each byte, so a bit shifted out of one channel
//    cannot bleed into the next channel.

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace raycaster {

constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 480;
constexpr int kMapSize = 24;
constexpr int kTexSize = 64;  // power of two; texY wrap uses & (kTexSize - 1)
constexpr int kMaxDdaSteps = 10000;
constexpr int kDarkenMask = 8355711;  // 0x7F7F7F, see header comment
constexpr double kInf = std::numeric_limits<double>::infinity();

// One cell of the world: 0 = empty, >0 = wall (and texture number).
struct World {
  // grid[y][x]; parsed from 24 digit rows. at(x, y) is the DDA view.
  std::vector<std::vector<int>> grid;
  int at(int x, int y) const { return grid[y][x]; }
  bool isWall(int x, int y) const {
    return x >= 0 && x < kMapSize && y >= 0 && y < kMapSize && at(x, y) > 0;
  }
};

inline World parseMap(const std::vector<std::string>& rows) {
  World w;
  for (const auto& row : rows) {
    std::vector<int> line;
    for (char c : row) line.push_back(c - '0');
    w.grid.push_back(line);
  }
  return w;
}

// Border-walled room with pillars, an inner room and corridors.
inline World untexturedMap() {
  return parseMap({
      "111111111111111111111111",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000001000000000000001",
      "100000001000000111110001",
      "100000101000000100010001",
      "100000001000000100010001",
      "100000001000000100010001",
      "100000001000000111110001",
      "100000001000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000011111111000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000100000000000000101",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "111111111111111111111111",
  });
}

// Same layout family, different arrangement, wall numbers 1-8 so
// each texture is reachable. Border is 1, interior walls 2-8.
inline World texturedMap() {
  return parseMap({
      "111111111111111111111111",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000002000000000000001",
      "100000002000000777770001",
      "100000402000000700080001",
      "100000002000000700080001",
      "100000002000000700080001",
      "100000002000000777770001",
      "100000002000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000033333333000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000500000000000000601",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "100000000000000000000001",
      "111111111111111111111111",
  });
}

struct Player {
  double posX = 12.5, posY = 12.5;
  double dirX = -1.0, dirY = 0.0;    // looking west
  double planeX = 0.0, planeY = -0.66;  // perpendicular to dir, |plane| 0.66
};

struct Stripe {
  int mapX = 0, mapY = 0;
  int side = 0;  // 0 = x-side hit, 1 = y-side hit
  int wallNum = 0;
  double perpWallDist = 0.0;
  int lineHeight = 0;
  int drawStart = 0, drawEnd = 0;
  int steps = 0;
  bool hit = false;
  std::vector<std::pair<int, int>> trace;  // cells stepped through, for tests
};

// Cast one stripe. x is the screen column, 0 <= x < kScreenWidth.
inline Stripe castStripe(const World& world, const Player& p, int x) {
  Stripe s;
  const double cameraX = 2.0 * x / kScreenWidth - 1.0;
  const double rayDirX = p.dirX + p.planeX * cameraX;
  const double rayDirY = p.dirY + p.planeY * cameraX;

  int mapX = static_cast<int>(p.posX);
  int mapY = static_cast<int>(p.posY);
  // Simplified deltas (see header comment): |1/rayDir| per axis.
  const double deltaX = (rayDirX == 0.0) ? kInf : std::fabs(1.0 / rayDirX);
  const double deltaY = (rayDirY == 0.0) ? kInf : std::fabs(1.0 / rayDirY);
  const int stepX = (rayDirX < 0.0) ? -1 : 1;
  const int stepY = (rayDirY < 0.0) ? -1 : 1;
  double sideX = (rayDirX < 0.0) ? (p.posX - mapX) * deltaX
                                : (mapX + 1.0 - p.posX) * deltaX;
  double sideY = (rayDirY < 0.0) ? (p.posY - mapY) * deltaY
                                : (mapY + 1.0 - p.posY) * deltaY;

  int side = 0;
  bool hit = false;
  while (!hit && s.steps < kMaxDdaSteps) {
    if (sideX < sideY) {
      sideX += deltaX;
      mapX += stepX;
      side = 0;
    } else {
      sideY += deltaY;
      mapY += stepY;
      side = 1;
    }
    ++s.steps;
    s.trace.push_back({mapX, mapY});
    if (world.isWall(mapX, mapY)) hit = true;
  }
  s.mapX = mapX;
  s.mapY = mapY;
  s.side = side;
  s.hit = hit;
  if (!hit) return s;
  s.wallNum = world.at(mapX, mapY);
  s.perpWallDist = (side == 0) ? (sideX - deltaX) : (sideY - deltaY);
  s.lineHeight = static_cast<int>(kScreenHeight / s.perpWallDist);
  s.drawStart = std::max(0, -s.lineHeight / 2 + kScreenHeight / 2);
  s.drawEnd = std::min(kScreenHeight - 1, s.lineHeight / 2 + kScreenHeight / 2);
  return s;
}

inline int darken(int colour) { return (colour >> 1) & kDarkenMask; }

// Untextured wall palette, indexed by wall number (1-based, 0 unused).
inline int wallColour(int wallNum) {
  static const int palette[] = {0x000000, 0xFF0000, 0x00FF00, 0x0000FF,
                                0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF,
                                0xFF8000};
  if (wallNum < 1 || wallNum > 8) return 0x808080;
  return palette[wallNum];
}

// 8 procedural 64x64 packed-RGB textures, indexed by wallNum - 1.
// Each is a distinct formula (checker / stripes / grid / dots /
// cross / gradient / rings / diagonal), generated, never loaded.
inline int texel(int texNum, int texX, int texY) {
  const int x = texX & (kTexSize - 1), y = texY & (kTexSize - 1);
  switch (texNum) {
    case 0: {  // red checker
      const bool c = ((x / 8) + (y / 8)) % 2 == 0;
      return c ? 0xC00000 : 0x400000;
    }
    case 1: {  // green vertical stripes
      return (x / 8) % 2 == 0 ? 0x00C000 : 0x004000;
    }
    case 2: {  // blue horizontal stripes
      return (y / 8) % 2 == 0 ? 0x0000C0 : 0x000040;
    }
    case 3: {  // yellow grid lines on dark
      return (x % 16 == 0 || y % 16 == 0) ? 0xC0C000 : 0x202000;
    }
    case 4: {  // magenta dots
      const bool dot = (x % 16 < 8) && (y % 16 < 8);
      return dot ? 0xC000C0 : 0x200020;
    }
    case 5: {  // cyan cross
      return (x == 32 || y == 32) ? 0x00C0C0 : 0x002020;
    }
    case 6: {  // grey gradient by x
      const int v = (x * 255) / 63;
      return (v << 16) | (v << 8) | v;
    }
    default: {  // orange diagonal bands
      return ((x + y) / 16) % 2 == 0 ? 0xC08000 : 0x402000;
    }
  }
}

// Framebuffer: kScreenHeight rows x kScreenWidth cols, packed RGB.
using Buffer = std::vector<int>;

inline Buffer renderFrame(const World& world, const Player& p, bool textured) {
  Buffer buf(kScreenWidth * kScreenHeight, 0x000000);
  for (int x = 0; x < kScreenWidth; ++x) {
    const Stripe s = castStripe(world, p, x);
    if (!s.hit) continue;
    if (!textured) {
      int colour = wallColour(s.wallNum);
      if (s.side == 1) colour = darken(colour);
      for (int y = s.drawStart; y <= s.drawEnd; ++y) buf[y * kScreenWidth + x] = colour;
      continue;
    }
    // Textured column (DESIGN): wallX -> texX, affine vertical step.
    const double cameraX = 2.0 * x / kScreenWidth - 1.0;
    const double rayDirX = p.dirX + p.planeX * cameraX;
    const double rayDirY = p.dirY + p.planeY * cameraX;
    double wallX = (s.side == 0) ? p.posY + s.perpWallDist * rayDirY
                                 : p.posX + s.perpWallDist * rayDirX;
    wallX -= std::floor(wallX);
    const int texX = static_cast<int>(wallX * kTexSize);
    const double step = static_cast<double>(kTexSize) / s.lineHeight;
    double texPos = (s.drawStart - kScreenHeight / 2 + s.lineHeight / 2) * step;
    for (int y = s.drawStart; y <= s.drawEnd; ++y) {
      // texY wrap only valid because kTexSize is a power of two.
      const int texY = static_cast<int>(texPos) & (kTexSize - 1);
      texPos += step;
      int colour = texel(s.wallNum - 1, texX, texY);
      if (s.side == 1) colour = darken(colour);
      buf[y * kScreenWidth + x] = colour;
    }
  }
  return buf;
}

// Movement: speeds are per second, multiplied by frameTime, so
// distance/angle scale linearly with frameTime (TEST_PLAN check).
// Collision is per axis (x then y), point check only. Outside
// the map counts as a wall: a raw grid index from a negative
// coordinate would be out of bounds (C++ truncates toward zero),
// so canStand bounds-checks first. This was caught by the
// collision test during development.
inline bool canStand(const World& w, double x, double y) {
  const int cx = static_cast<int>(x), cy = static_cast<int>(y);
  return cx >= 0 && cx < kMapSize && cy >= 0 && cy < kMapSize && w.at(cx, cy) == 0;
}
inline void moveForward(Player& p, const World& w, double frameTime, double speed = 5.0) {
  const double nx = p.posX + p.dirX * speed * frameTime;
  if (canStand(w, nx, p.posY)) p.posX = nx;
  const double ny = p.posY + p.dirY * speed * frameTime;
  if (canStand(w, p.posX, ny)) p.posY = ny;
}
inline void moveBackward(Player& p, const World& w, double frameTime, double speed = 5.0) {
  const double nx = p.posX - p.dirX * speed * frameTime;
  if (canStand(w, nx, p.posY)) p.posX = nx;
  const double ny = p.posY - p.dirY * speed * frameTime;
  if (canStand(w, p.posX, ny)) p.posY = ny;
}
inline void rotate(Player& p, double angle) {
  const double c = std::cos(angle), s = std::sin(angle);
  const double oldDirX = p.dirX;
  p.dirX = p.dirX * c - p.dirY * s;
  p.dirY = oldDirX * s + p.dirY * c;
  const double oldPlaneX = p.planeX;
  p.planeX = p.planeX * c - p.planeY * s;
  p.planeY = oldPlaneX * s + p.planeY * c;
}

// FPS from frame time; frameTime == 0 yields 0, never a crash.
inline double fps(double frameTime) { return frameTime > 0.0 ? 1.0 / frameTime : 0.0; }

}  // namespace raycaster
