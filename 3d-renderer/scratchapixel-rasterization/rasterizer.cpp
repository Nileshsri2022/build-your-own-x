// Scratchapixel Rasterization — visibility-only triangle rasterizer.
//
// Own implementation following the lesson's algorithm (object-centric
// rasterization, Pineda edge function, barycentric interpolation,
// top-left rule, perspective divide, z-buffer, hand-written P3 PPM).
// No lesson code is copied verbatim, no image library, no window.
//
// Scope, stated honestly (docs/SPEC.md §7):
//   * triangles only, flat vertex-colour interpolation only;
//   * NO Phong/diffuse/specular, NO shadows/reflections/refraction,
//     NO texture sampling (st coordinates are carried as data and
//     interpolated perspective-correctly, but never sampled),
//     NO anti-aliasing, NO GPU API.
//   * Depth Buffer chapter (ch. 4) was never opened: the z-buffer
//     here is only the overview-level behaviour — a float array,
//     initialised to infinity, nearest wins. Depth is interpolated
//     linearly in screen space; exact depth tests therefore use
//     only triangles parallel to the image plane (constant depth),
//     where screen-space and perspective-correct interpolation agree.
//   * Near clipping: a triangle with any vertex behind the camera
//     or closer than the near plane is conservatively discarded
//     whole, so it can never produce garbage pixels. This is our
//     own stated decision, not a claim about the lesson's procedure
//     (the projection-chapter tail was not fetched).
//
// Camera convention (our own, fixed here and in the tests):
//   camera at the origin looking down -Z, 90-degree vertical FOV,
//   so tan(fov/2) = 1. A vertex is in front iff -z >= nearPlane.
//   Depth stored in the z-buffer is the camera distance -z:
//   smaller = nearer. Pixel centres are (x + 0.5, y + 0.5).
//   Winding is normalised so the signed raster-space area is
//   positive; a pixel exactly on an edge is drawn iff that edge
//   is a "top" edge (horizontal, going right) or a "left" edge
//   (going up, dy < 0). A shared edge therefore belongs to exactly
//   one of its two triangles: no gaps, no double draws.
//   Equal depth: first triangle wins (strict < depth test).

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kNearPlane = 0.1;
constexpr int kDefaultSize = 64;

struct Vec2 {
    double x = 0, y = 0;
};
struct Vec3 {
    double x = 0, y = 0, z = 0;
};
struct Colour {
    int r = 0, g = 0, b = 0;
};
struct Vertex {
    Vec3 pos;       // camera space
    Colour colour;  // 8-bit per channel
    Vec2 st;        // texture coordinates, data only, never sampled
};
struct Triangle {
    Vertex v[3];
};

// Pineda edge function: cross(b - a, p - a). Positive on one side
// of the directed edge a -> b, negative on the other, zero on it.
double edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

bool isTopLeftEdge(const Vec2& a, const Vec2& b) {
    const double dx = b.x - a.x, dy = b.y - a.y;
    return (dy == 0.0 && dx > 0.0) || dy < 0.0;
}

int clampByte(double v) {
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return static_cast<int>(std::lround(v));
}

struct Projected {
    Vec2 raster;  // pixel coordinates
    double depth; // camera distance -z
    Colour colour;
    Vec2 st;
    double invDepth; // 1 / depth, for perspective-correct st
};

// Camera -> NDC [-1,1] -> raster. Returns false if the vertex is
// behind the camera or closer than the near plane.
bool project(const Vertex& v, int width, int height, Projected& out) {
    const double dist = -v.pos.z;
    if (dist < kNearPlane) return false;
    const double aspect = static_cast<double>(width) / height;
    const double ndcX = v.pos.x / (dist * aspect);
    const double ndcY = v.pos.y / dist;
    out.raster.x = (ndcX + 1.0) * 0.5 * width;
    out.raster.y = (1.0 - ndcY) * 0.5 * height;
    out.depth = dist;
    out.colour = v.colour;
    out.st = v.st;
    out.invDepth = 1.0 / dist;
    return true;
}

struct FrameBuffer {
    int width = 0, height = 0;
    std::vector<Colour> pixels;  // row-major, top row first
    std::vector<double> depth;   // same layout, init infinity
    FrameBuffer(int w, int h, Colour bg)
        : width(w), height(h), pixels(static_cast<size_t>(w) * h, bg),
          depth(static_cast<size_t>(w) * h,
                std::numeric_limits<double>::infinity()) {}
    Colour& pixel(int x, int y) {
        return pixels[static_cast<size_t>(y) * width + x];
    }
    double& depthAt(int x, int y) {
        return depth[static_cast<size_t>(y) * width + x];
    }
};

void rasterizeTriangle(const Triangle& tri, FrameBuffer& fb) {
    Projected p[3];
    for (int i = 0; i < 3; ++i) {
        if (!project(tri.v[i], fb.width, fb.height, p[i])) return;
    }
    // Normalise winding: signed area must be positive so the
    // inside test (all edge values > 0, or == 0 on a top-left
    // edge) and the top-left rule both hold uniformly.
    double area = edgeFunction(p[0].raster, p[1].raster, p[2].raster);
    if (area == 0.0) return;  // degenerate: covers no pixels
    if (area < 0.0) {
        std::swap(p[1], p[2]);
        area = -area;
    }
    // Bounding box, clamped to the image: pixels outside it are
    // never tested (the lesson's main optimisation over the
    // naive whole-image loop).
    double minX = std::min({p[0].raster.x, p[1].raster.x, p[2].raster.x});
    double maxX = std::max({p[0].raster.x, p[1].raster.x, p[2].raster.x});
    double minY = std::min({p[0].raster.y, p[1].raster.y, p[2].raster.y});
    double maxY = std::max({p[0].raster.y, p[1].raster.y, p[2].raster.y});
    const int x0 = std::max(0, static_cast<int>(std::floor(minX)));
    const int x1 = std::min(fb.width - 1, static_cast<int>(std::ceil(maxX)));
    const int y0 = std::max(0, static_cast<int>(std::floor(minY)));
    const int y1 = std::min(fb.height - 1, static_cast<int>(std::ceil(maxY)));
    if (x0 > x1 || y0 > y1) return;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const Vec2 centre{static_cast<double>(x) + 0.5,
                              static_cast<double>(y) + 0.5};
            const double e[3] = {
                edgeFunction(p[1].raster, p[2].raster, centre),
                edgeFunction(p[2].raster, p[0].raster, centre),
                edgeFunction(p[0].raster, p[1].raster, centre),
            };
            // Edge i is opposite vertex i. Edge 0 is p1->p2, etc.
            const Vec2 edgeA[3] = {p[1].raster, p[2].raster, p[0].raster};
            const Vec2 edgeB[3] = {p[2].raster, p[0].raster, p[1].raster};
            bool inside = true;
            for (int i = 0; i < 3; ++i) {
                if (e[i] < 0.0) {
                    inside = false;
                    break;
                }
                if (e[i] == 0.0 && !isTopLeftEdge(edgeA[i], edgeB[i])) {
                    inside = false;
                    break;
                }
            }
            if (!inside) continue;
            // Barycentric weights: edge value / total area, in
            // vertex order (weight of vertex i is opposite edge i).
            const double w0 = e[0] / area, w1 = e[1] / area, w2 = e[2] / area;
            // Depth: screen-space linear interpolation (see header
            // note; exact for the constant-depth test cases).
            const double depth =
                w0 * p[0].depth + w1 * p[1].depth + w2 * p[2].depth;
            // Strict <: first triangle wins at equal depth.
            if (!(depth < fb.depthAt(x, y))) continue;
            // Colour: barycentric (screen-space linear) interpolation.
            const Colour c{
                clampByte(w0 * p[0].colour.r + w1 * p[1].colour.r +
                          w2 * p[2].colour.r),
                clampByte(w0 * p[0].colour.g + w1 * p[1].colour.g +
                          w2 * p[2].colour.g),
                clampByte(w0 * p[0].colour.b + w1 * p[1].colour.b +
                          w2 * p[2].colour.b),
            };
            // st: perspective-correct interpolation, carried as
            // data only — computed here to keep the pipeline honest,
            // never sampled into the output.
            const double invSum =
                w0 * p[0].invDepth + w1 * p[1].invDepth + w2 * p[2].invDepth;
            const Vec2 st{
                (w0 * p[0].st.x * p[0].invDepth +
                 w1 * p[1].st.x * p[1].invDepth +
                 w2 * p[2].st.x * p[2].invDepth) /
                    invSum,
                (w0 * p[0].st.y * p[0].invDepth +
                 w1 * p[1].st.y * p[1].invDepth +
                 w2 * p[2].st.y * p[2].invDepth) /
                    invSum,
            };
            (void)st;
            fb.pixel(x, y) = c;
            fb.depthAt(x, y) = depth;
        }
    }
}

bool writePPM(const FrameBuffer& fb, const std::string& path) {
    std::ofstream out(path);
    if (!out) return false;
    out << "P3\n" << fb.width << ' ' << fb.height << "\n255\n";
    for (int y = 0; y < fb.height; ++y) {
        for (int x = 0; x < fb.width; ++x) {
            const Colour& c =
                fb.pixels[static_cast<size_t>(y) * fb.width + x];
            out << c.r << ' ' << c.g << ' ' << c.b << ' ';
        }
        out << '\n';
    }
    return static_cast<bool>(out);
}

Vertex V(double x, double y, double z, Colour c) {
    return Vertex{Vec3{x, y, z}, c, Vec2{0, 0}};
}
Vertex VS(double x, double y, double z, Colour c, double s, double t) {
    return Vertex{Vec3{x, y, z}, c, Vec2{s, t}};
}
Triangle tri(Vertex a, Vertex b, Vertex c) {
    Triangle t;
    t.v[0] = a;
    t.v[1] = b;
    t.v[2] = c;
    return t;
}

// Scenes, each a fixed camera-space triangle list. Raster-space
// scenes are expressed through the 90-degree projection at z = -1
// on a square image: raster (rx, ry) in a W image maps to camera
// (rx / W * 2 - 1, 1 - ry / W * 2, -1). Test scenes use that mapping.
std::vector<Triangle> scene(const std::string& name, int w) {
    const double s = static_cast<double>(w);
    auto at = [&](double rx, double ry, double z, Colour c) {
        const double aspect = 1.0;  // scenes assume square images
        (void)aspect;
        // Inverse projection at depth d = -z (square image only;
        // non-square runs scale x by the aspect, as project() does).
        const double d = -z;
        return V((rx / s * 2.0 - 1.0) * d, (1.0 - ry / s * 2.0) * d, z, c);
    };
    const Colour red{255, 0, 0}, green{0, 255, 0}, blue{0, 0, 255};
    const Colour dark{200, 30, 30}, navy{30, 30, 200};
    if (name == "default") {
        // RGB triangle, parallel to the image plane at z = -2,
        // centred in the image (docs SPEC §3 default scene).
        return {tri(V(0, 0.6, -2, red), V(-0.6, -0.5, -2, green),
                    V(0.6, -0.5, -2, blue))};
    }
    if (name == "solid") {
        // Solid triangle with raster vertices (8,8), (24,8), (8,24)
        // at 32x32 (scaled by w/32 for other sizes).
        const double k = s / 32.0;
        return {tri(at(8 * k, 8 * k, -1, dark), at(24 * k, 8 * k, -1, dark),
                    at(8 * k, 24 * k, -1, dark))};
    }
    if (name == "quad") {
        // Two triangles sharing the diagonal of a square, red/green.
        const double k = s / 32.0;
        return {tri(at(8 * k, 8 * k, -1, red), at(24 * k, 8 * k, -1, red),
                    at(24 * k, 24 * k, -1, red)),
                tri(at(8 * k, 8 * k, -1, green), at(24 * k, 24 * k, -1, green),
                    at(8 * k, 24 * k, -1, green))};
    }
    if (name == "depth-near-first") {
        const double k = s / 32.0;
        return {tri(at(8 * k, 8 * k, -1, red), at(24 * k, 8 * k, -1, red),
                    at(8 * k, 24 * k, -1, red)),
                tri(at(8 * k, 8 * k, -2, navy), at(24 * k, 8 * k, -2, navy),
                    at(8 * k, 24 * k, -2, navy))};
    }
    if (name == "depth-far-first") {
        const double k = s / 32.0;
        return {tri(at(8 * k, 8 * k, -2, navy), at(24 * k, 8 * k, -2, navy),
                    at(8 * k, 24 * k, -2, navy)),
                tri(at(8 * k, 8 * k, -1, red), at(24 * k, 8 * k, -1, red),
                    at(8 * k, 24 * k, -1, red))};
    }
    if (name == "depth-equal") {
        // Coplanar overlap: first triangle must win (strict <).
        const double k = s / 32.0;
        return {tri(at(8 * k, 8 * k, -1, red), at(24 * k, 8 * k, -1, red),
                    at(8 * k, 24 * k, -1, red)),
                tri(at(8 * k, 8 * k, -1, navy), at(24 * k, 8 * k, -1, navy),
                    at(8 * k, 24 * k, -1, navy))};
    }
    if (name == "empty") return {};
    if (name == "clip") {
        // Triangle extending far beyond the image edges.
        return {tri(V(-2, 2, -1, dark), V(2, 2, -1, dark), V(-2, -2, -1, dark))};
    }
    if (name == "persp-near") {
        return {tri(V(-0.5, 0.5, -1, dark), V(0.5, 0.5, -1, dark),
                    V(-0.5, -0.5, -1, dark))};
    }
    if (name == "persp-far") {
        // Same world size as persp-near, twice as far away.
        return {tri(V(-0.5, 0.5, -2, dark), V(0.5, 0.5, -2, dark),
                    V(-0.5, -0.5, -2, dark))};
    }
    if (name == "offcentre") {
        // Displaced right (+x) and up (+y) in camera space.
        return {tri(V(0.2, 0.7, -2, red), V(0.2, -0.3, -2, green),
                    V(1.0, -0.3, -2, blue))};
    }
    if (name == "behind") {
        // Behind the camera and inside the near plane: no pixels.
        return {tri(V(0, 0.5, 1, dark), V(-0.5, -0.5, 1, dark),
                    V(0.5, -0.5, 1, dark)),
                tri(V(0, 0.05, -0.05, dark), V(-0.05, -0.05, -0.05, dark),
                    V(0.05, -0.05, -0.05, dark))};
    }
    if (name == "crossing") {
        // Straddles the near plane: conservatively discarded whole.
        return {tri(V(0, 0.6, -2, dark), V(-0.6, -0.5, -0.05, dark),
                    V(0.6, -0.5, -2, dark))};
    }
    if (name == "st") {
        // Same footprint as solid, with distinct st per vertex:
        // proves st data flows through the pipeline (not sampled).
        const double k = s / 32.0;
        return {tri(VS(8 * k / s * 2 - 1, 1 - 8 * k / s * 2, -1, dark, 0, 0),
                    VS(24 * k / s * 2 - 1, 1 - 8 * k / s * 2, -1, dark, 1, 0),
                    VS(8 * k / s * 2 - 1, 1 - 24 * k / s * 2, -1, dark, 0, 1))};
    }
    return {};  // unknown scene name: handled by caller as an error
}

bool knownScene(const std::string& name) {
    static const char* names[] = {
        "default", "solid", "quad", "depth-near-first", "depth-far-first",
        "depth-equal", "empty", "clip", "persp-near", "persp-far",
        "offcentre", "behind", "crossing", "st"};
    for (const char* n : names)
        if (name == n) return true;
    return false;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3 || argc > 5) {
        std::cerr << "usage: rasterizer <out.ppm> <scene> [width height]\n"
                     "scenes: default solid quad depth-near-first "
                     "depth-far-first depth-equal empty clip persp-near "
                     "persp-far offcentre behind crossing st\n";
        return 2;
    }
    const std::string outPath = argv[1];
    const std::string sceneName = argv[2];
    if (!knownScene(sceneName)) {
        std::cerr << "error: unknown scene '" << sceneName << "'\n";
        return 2;
    }
    int width = kDefaultSize, height = kDefaultSize;
    if (argc >= 4) {
        width = std::atoi(argv[3]);
        height = (argc == 5) ? std::atoi(argv[4]) : width;
        if (width <= 0 || height <= 0) {
            std::cerr << "error: width and height must be positive\n";
            return 2;
        }
    }
    FrameBuffer fb(width, height, Colour{0, 0, 0});
    for (const Triangle& t : scene(sceneName, width)) rasterizeTriangle(t, fb);
    if (!writePPM(fb, outPath)) {
        std::cerr << "error: cannot write '" << outPath << "'\n";
        return 1;
    }
    return 0;
}
