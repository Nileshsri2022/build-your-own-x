// Unit tests for the Scratchapixel raytracer (TEST_PLAN, assert-style).
// Exit 0 only if every check passes; each failure prints its name.
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../src/raytracer.hpp"

using namespace raytracer;

static int failures = 0;
static void check(bool ok, const std::string& name) {
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << name << '\n';
    } else {
        std::cout << "ok: " << name << '\n';
    }
}
static bool near(double a, double b, double tol = 1e-6) { return std::fabs(a - b) <= tol; }
static bool colorNear(Color a, Color b, double tol = 3.0 / 255) {
    return near(a.x, b.x, tol) && near(a.y, b.y, tol) && near(a.z, b.z, tol);
}

int main() {
    // --- Vectors ---
    Vec3 a{1, 2, 3}, b{4, -1, 0};
    check((a + b).x == 5 && (a + b).y == 1 && (a + b).z == 3, "vec add");
    check((a - b).x == -3 && (a - b).y == 3 && (a - b).z == 3, "vec sub");
    check((a * 2).x == 2 && (a * 2).y == 4 && (a * 2).z == 6, "vec scale");
    check(near(Vec3{1, 0, 0}.dot(Vec3{0, 1, 0}), 0), "dot perpendicular = 0");
    check(near(Vec3{2, 0, 0}.dot(Vec3{3, 0, 0}), 6), "dot parallel = product");
    check(near(Vec3{3, 4, 0}.length(), 5), "vec length 3-4-5");
    check(near(Vec3{3, 4, 0}.normalized().length(), 1), "normalize unit length");
    Vec3 zero = Vec3{0, 0, 0}.normalized();
    check(zero.x == 0 && zero.y == 0 && zero.z == 0, "zero normalize -> (0,0,0), no NaN");

    // --- Sphere intersection ---
    Sphere s{{0, 0, -5}, 1, {1, 0, 0}, Material::Opaque};
    auto hit = intersectSphere(Ray{{0, 0, 0}, {0, 0, -1}}, s);
    check(hit && near(hit->t, 4.0), "sphere centre ray hits at t=4");
    check(!intersectSphere(Ray{{0, 0, 0}, {0, 1, 0}}, s), "sphere miss to the side");
    auto tangent = intersectSphere(Ray{{1, 0, 0}, {0, 0, -1}}, s);
    check(tangent && near(tangent->t, 5.0, 1e-6), "sphere tangent: one hit at t=5");
    auto inside = intersectSphere(Ray{{0, 0, -5}, {0, 0, -1}}, s);
    check(inside && near(inside->t, 1.0), "ray inside sphere exits at t=1");
    check(!intersectSphere(Ray{{0, 0, 0}, {0, 0, 1}}, s), "ray pointing away: no hit");
    // Nearest hit: nearer sphere wins.
    Scene two;
    two.spheres = {{{0, 0, -8}, 1, {0, 0, 1}, Material::Opaque},
                   {{0, 0, -5}, 1, {1, 0, 0}, Material::Opaque}};
    auto nearHit = two.nearestHit(Ray{{0, 0, 0}, {0, 0, -1}});
    check(nearHit && near(nearHit->t, 4.0) && nearHit->sphere->color.x == 1,
          "nearest hit wins");
    // Normal: outward, unit, at a known hit point.
    check(hit && near(hit->normal.length(), 1) && near(hit->normal.z, 1) &&
              near(hit->point.z, -4.0),
          "normal outward unit at front hit point");

    // --- Shading / shadow (custom scenes, single light) ---
    Scene lit;
    lit.spheres = {{{0, 0, -5}, 1, {1, 0, 0}, Material::Opaque}};
    lit.lights = {{{5, 5, 0}, 1.0}};
    Color litPx = trace(lit, Ray{{0, 0, 0}, {0, 0, -1}}, 0);
    check(colorNear(litPx, {1, 0, 0}), "lit pixel = object colour x brightness");
    Scene shadowed = lit;
    shadowed.spheres.push_back({{2.5, 2.5, -2.5}, 1, {0, 1, 0}, Material::Opaque});
    Color shadowPx = trace(shadowed, Ray{{0, 0, 0}, {0, 0, -1}}, 0);
    check(colorNear(shadowPx, {0, 0, 0}), "blocked light contributes 0 (black)");

    // --- Reflection: reflective sphere facing a yellow sphere behind camera ---
    Scene refl;
    refl.spheres = {{{0, 0, -5}, 1, {0, 1, 0}, Material::Reflective},
                    {{0, 0, 2}, 2, {1, 1, 0}, Material::Opaque}};
    refl.lights = {{{5, 5, 0}, 1.0}};
    Color reflPx = trace(refl, Ray{{0, 0, 0}, {0, 0, -1}}, 0);
    check(reflPx.x > 0.4 && reflPx.y > 0.9,
          "reflective pixel picks up reflected yellow (R>0.4, not flat green)");

    // --- Fresnel / glass blend structure ---
    check(near(fresnelSchlick(1.0, 1.5), 0.04, 0.005), "Fresnel head-on ~= 0.04");
    check(near(fresnelSchlick(0.0, 1.5), 1.0), "Fresnel grazing = 1.0");
    Scene glass;
    glass.spheres = {{{0, 0, -5}, 1, {0, 0, 1}, Material::Glass}};
    glass.lights = {{{5, 5, 0}, 1.0}};
    Color glassPx = trace(glass, Ray{{0, 0, 0}, {0, 0, -1}}, 0);
    check(!colorNear(glassPx, {0, 0, 1}) && glassPx.z < 0.2,
          "glass head-on pixel is not flat blue (mostly refracted to black bg)");

    // --- Depth limit: two facing reflective spheres must terminate ---
    Scene facing;
    facing.spheres = {{{0, 0, -5}, 1, {0, 1, 0}, Material::Reflective},
                      {{0, 0, 3}, 2, {1, 0, 0}, Material::Reflective}};
    facing.lights = {{{5, 5, 0}, 1.0}};
    Color facingPx = trace(facing, Ray{{0, 0, 0}, {0, 0, -1}}, 0);
    check(facingPx.x >= 0 && facingPx.x <= 1 && facingPx.y >= 0 && facingPx.y <= 1,
          "facing reflective spheres terminate (depth limit)");

    // --- Empty scene renders background only ---
    Scene empty;
    Image emptyImg = render(empty, 4, 3);
    bool allBlack = true;
    for (const auto& px : emptyImg.pixels)
        if (px.x != 0 || px.y != 0 || px.z != 0) allBlack = false;
    check(allBlack && emptyImg.width == 4 && emptyImg.height == 3, "empty scene all black");

    // --- PPM writer: exact header, exact integer count, known bytes ---
    Image tiny{2, 2, {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}}};
    check(writePpm("/tmp/scratchapixel-test.ppm", tiny), "PPM write succeeds");
    std::ifstream in("/tmp/scratchapixel-test.ppm");
    std::stringstream ss;
    ss << in.rdbuf();
    check(ss.str() == "P3\n2 2\n255\n255 0 0\n0 255 0\n0 0 255\n255 255 255\n",
          "PPM 2x2 exact bytes");
    Image one{1, 1, {{0.5, 0.5, 0.5}}};
    check(writePpm("/tmp/scratchapixel-test1.ppm", one), "PPM 1x1 write succeeds");
    std::ifstream in1("/tmp/scratchapixel-test1.ppm");
    std::stringstream ss1;
    ss1 << in1.rdbuf();
    check(ss1.str() == "P3\n1 1\n255\n128 128 128\n", "PPM 1x1 exact bytes (round 0.5*255)");

    if (failures == 0) {
        std::cout << "ALL UNIT TESTS PASSED\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed\n";
    return 1;
}
