// Assert-based tests for the RTOW Book 1 renderer: header units plus
// scripted PPM pixel checks on real renders through camera::render.
// Tolerances, not exact bytes: the renderer samples randomly.
// Exit code 0 = all pass; any failure prints and aborts.

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

#include "../camera.h"
#include "../hittable_list.h"
#include "../sphere.h"

static int failures = 0;

#define CHECK(cond)                                        \
    do {                                                   \
        if (!(cond)) {                                     \
            std::cerr << "FAIL line " << __LINE__ << ": " \
                      << #cond << "\n";                    \
            failures++;                                    \
        }                                                  \
    } while (0)

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

struct ppm {
    int w = 0, h = 0;
    std::vector<std::array<int, 3>> px;  // row-major, top row first
    std::array<int, 3> at(int i, int j) const { return px[j * w + i]; }
};

// Render a world through a camera, capturing stdout (PPM) and
// silencing the stderr progress line.
static ppm render_ppm(const hittable& world, double aspect, int width,
                      int samples, int depth = 50, double vfov = 90,
                      point3 lookfrom = point3(0, 0, 0),
                      point3 lookat = point3(0, 0, -1),
                      double defocus_angle = 0, double focus_dist = 10) {
    camera cam;
    cam.aspect_ratio = aspect;
    cam.image_width = width;
    cam.samples_per_pixel = samples;
    cam.max_depth = depth;
    cam.vfov = vfov;
    cam.lookfrom = lookfrom;
    cam.lookat = lookat;
    cam.vup = vec3(0, 1, 0);
    cam.defocus_angle = defocus_angle;
    cam.focus_dist = focus_dist;

    std::stringstream buf;
    std::stringstream null_log;
    auto old_out = std::cout.rdbuf(buf.rdbuf());
    auto old_log = std::clog.rdbuf(null_log.rdbuf());
    cam.render(world);
    std::cout.rdbuf(old_out);
    std::clog.rdbuf(old_log);

    ppm out;
    std::string magic;
    int maxval;
    buf >> magic >> out.w >> out.h >> maxval;
    CHECK(magic == "P3");
    CHECK(maxval == 255);
    out.px.resize(out.w * out.h);
    for (int k = 0; k < out.w * out.h; k++) {
        int r, g, b;
        CHECK((bool)(buf >> r >> g >> b));
        CHECK(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 &&
              b <= 255);
        out.px[k] = {r, g, b};
    }
    return out;
}

// Mean colour of a 5x5 patch around (cx, cy): diffuse pixels are
// noisy single-pixel, so structural comparisons use patch means.
static std::array<double, 3> patch_mean(const ppm& p, int cx, int cy) {
    double s[3] = {0, 0, 0};
    int n = 0;
    for (int j = cy - 2; j <= cy + 2; j++)
        for (int i = cx - 2; i <= cx + 2; i++) {
            auto px = p.at(i, j);
            for (int c = 0; c < 3; c++)
                s[c] += px[c];
            n++;
        }
    return {s[0] / n, s[1] / n, s[2] / n};
}

static void test_vec3_ray_interval() {
    vec3 a(1, 2, 3), b(4, -5, 6);
    CHECK((a + b).x() == 5 && (a + b).y() == -3);
    CHECK((a - b).x() == -3 && (a - b).z() == -3);
    CHECK((a * 2).y() == 4 && (a / 2).x() == 0.5);
    CHECK(dot(vec3(1, 0, 0), vec3(0, 1, 0)) == 0);
    CHECK(cross(vec3(1, 0, 0), vec3(0, 1, 0)).z() == 1);  // right-hand
    CHECK(near(vec3(3, 4, 0).length(), 5));
    CHECK(near(unit_vector(vec3(0, 3, 0)).length(), 1));
    CHECK(vec3(1e-9, 0, 0).near_zero());
    CHECK(!vec3(1e-7, 0, 0).near_zero());

    ray r(point3(1, 2, 3), vec3(0, 1, 0));
    CHECK(r.at(0).y() == 2 && r.at(1).y() == 3 && r.at(-1).y() == 1);

    interval iv(0.001, 10);
    CHECK(iv.contains(0.001) && iv.contains(10) && !iv.contains(0));
    CHECK(!iv.surrounds(0.001) && iv.surrounds(5) && !iv.surrounds(10));
    CHECK(iv.clamp(-5) == 0.001 && iv.clamp(99) == 10 && iv.clamp(5) == 5);
}

static void test_sphere_hit() {
    auto mat = make_shared<lambertian>(color(1, 0, 0));
    sphere s(point3(0, 0, -1), 0.5, mat);
    hit_record rec;

    // Straight at centre: t = 0.5 (front surface at z = -0.5).
    CHECK(s.hit(ray(point3(0, 0, 0), vec3(0, 0, -1)),
                interval(0.001, infinity), rec));
    CHECK(near(rec.t, 0.5));
    CHECK(rec.front_face);
    CHECK(near(rec.normal.z(), 1));

    // Missing to the side.
    CHECK(!s.hit(ray(point3(0, 0, 0), vec3(1, 0, 0)),
                 interval(0.001, infinity), rec));
    // Pointing away.
    CHECK(!s.hit(ray(point3(0, 0, 0), vec3(0, 0, 1)),
                 interval(0.001, infinity), rec));
    // Starting inside: exits at positive t, back face.
    CHECK(s.hit(ray(point3(0, 0, -1), vec3(0, 0, -1)),
                interval(0.001, infinity), rec));
    CHECK(!rec.front_face);
    CHECK(near(rec.t, 0.5));

    // Nearest root: a ray through the sphere reports the near root,
    // and a hittable_list picks the nearer of two spheres.
    hittable_list world;
    auto far = make_shared<sphere>(point3(0, 0, -3), 0.5,
                                   make_shared<lambertian>(color(0, 0, 1)));
    auto near_s = make_shared<sphere>(point3(0, 0, -1), 0.5, mat);
    world.add(far);
    world.add(near_s);
    CHECK(world.hit(ray(point3(0, 0, 0), vec3(0, 0, -1)),
                    interval(0.001, infinity), rec));
    CHECK(near(rec.t, 0.5));
}

static void test_material_scatter() {
    hit_record rec;
    rec.p = point3(0, 0, 0);
    rec.normal = vec3(0, 1, 0);
    rec.front_face = true;
    color att;
    ray scattered;

    lambertian lamb(color(1, 0, 0));
    CHECK(lamb.scatter(ray(point3(0, 1, 0), vec3(0, -1, 0)), rec, att,
                       scattered));
    CHECK(scattered.direction().length_squared() > 0);
    CHECK(att.x() == 1 && att.y() == 0);

    // Metal reflecting below the surface is absorbed (false).
    metal m(color(0.8, 0.8, 0.8), 0.0);
    CHECK(!m.scatter(ray(point3(0, 0, 0), vec3(0, 1, 0)), rec, att,
                     scattered));
    // Metal reflecting above the surface succeeds.
    CHECK(m.scatter(ray(point3(0, 1, 0), vec3(0, -1, 0)), rec, att,
                    scattered));

    // Dielectric from outside, head-on: scatters, non-zero direction.
    dielectric glass(1.5);
    rec.normal = vec3(0, 0, 1);
    CHECK(glass.scatter(ray(point3(0, 0, 1), vec3(0, 0, -1)), rec, att,
                        scattered));
    CHECK(scattered.direction().length_squared() > 0);
    CHECK(att.x() == 1 && att.y() == 1 && att.z() == 1);

    // Total internal reflection: inside glass (front_face false),
    // glancing ray -> reflected direction, not refracted.
    rec.front_face = false;
    vec3 in_dir = unit_vector(vec3(0.99, 0, 0.14));
    CHECK(glass.scatter(ray(point3(0, 0, 0), in_dir), rec, att, scattered));
    vec3 expected = reflect(in_dir, rec.normal);
    CHECK(near(scattered.direction().x(), expected.x(), 1e-9));
    CHECK(near(scattered.direction().z(), expected.z(), 1e-9));
}

static hittable_list single_sphere_scene(color albedo, double z = -1,
                                         double r = 0.5) {
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, z), r,
                                  make_shared<lambertian>(albedo)));
    return world;
}

static void test_sky_and_single_sphere() {
    hittable_list empty;
    ppm sky = render_ppm(empty, 16.0 / 9.0, 160, 10);
    CHECK(sky.w == 160 && sky.h == 90);
    auto top = sky.at(80, 0), bot = sky.at(80, 89);
    // Note: the blue channel is 1.0 at BOTH gradient endpoints
    // (white (1,1,1) -> blue (0.5,0.7,1.0)), so the gradient shows
    // in red/green, not blue. Top is bluer in the sense of lower
    // red/green relative to blue.
    CHECK(bot[0] - top[0] > 20);  // bottom whiter (redder)
    CHECK(bot[1] - top[1] > 20);  // bottom whiter (greener)
    CHECK(top[2] >= 250 && bot[2] >= 250);  // blue stays ~255
    auto left = sky.at(0, 45), right = sky.at(159, 45);
    for (int c = 0; c < 3; c++)
        CHECK(std::abs(left[c] - right[c]) <= 10);

    ppm sq = render_ppm(empty, 1.0, 100, 4);
    CHECK(sq.w == 100 && sq.h == 100);  // no width/height swap

    ppm one = render_ppm(single_sphere_scene(color(1, 0, 0)), 16.0 / 9.0,
                         160, 20);
    auto centre = one.at(80, 45);
    CHECK(centre[0] - centre[1] > 20 && centre[0] - centre[2] > 20);
    auto corner = one.at(0, 0);
    auto sky_corner = sky.at(0, 0);
    for (int c = 0; c < 3; c++)
        CHECK(std::abs(corner[c] - sky_corner[c]) <= 10);

    // Lambertian shading shape: the sphere is not a flat disc.
    // Measured on a sphere sitting on a ground plane (the TEST_PLAN
    // side-limb wording does not hold for a lone sphere in open
    // sky, where limb bounces still hit bright sky — verified by
    // debug renders: side limb 193 vs centre 193, no margin; and
    // a bright-yellow ground also washes the effect out).
    // With a dark ground, the bottom limb — whose bounces hit
    // the ground instead of the sky — is far darker than the
    // centre. Patch centred at y=68 stays inside the disc
    // (bottom edge ~72); y=70 would straddle the edge.
    hittable_list ground_scene;
    ground_scene.add(make_shared<sphere>(
        point3(0, -100.5, -1), 100.0,
        make_shared<lambertian>(color(0.2, 0.2, 0.2))));
    ground_scene.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<lambertian>(color(1, 0, 0))));
    ppm grounded = render_ppm(ground_scene, 16.0 / 9.0, 160, 50);
    auto centre_mean = patch_mean(grounded, 80, 45);
    auto limb_mean = patch_mean(grounded, 80, 68);
    double centre_bright =
        centre_mean[0] + centre_mean[1] + centre_mean[2];
    double limb_bright = limb_mean[0] + limb_mean[1] + limb_mean[2];
    std::cerr << "limb patch means: centre=" << centre_bright
              << " bottom-limb=" << limb_bright << "\n";
    CHECK(centre_bright - limb_bright > 20);
}

static void test_nearest_metal_dielectric() {
    // Nearer red sphere wins over farther blue sphere on same ray.
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -3), 0.5,
                                  make_shared<lambertian>(color(0, 0, 1))));
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5,
                                  make_shared<lambertian>(color(1, 0, 0))));
    ppm img = render_ppm(world, 16.0 / 9.0, 160, 20);
    auto centre = img.at(80, 45);
    CHECK(centre[0] - centre[2] > 20);

    // Metal sphere in front of sky: sky-tinted, not flat albedo.
    hittable_list metal_world;
    metal_world.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<metal>(color(0.8, 0.8, 0.8), 0.0)));
    ppm metal_img = render_ppm(metal_world, 16.0 / 9.0, 160, 20);
    auto mcentre = metal_img.at(80, 45);
    CHECK(mcentre[2] - mcentre[0] > 10);  // blue-ish reflection
    CHECK(mcentre[0] + mcentre[1] + mcentre[2] > 400);  // bright

    // Fuzzy metal blurs the reflection: edge pixels differ from
    // sharp metal beyond tolerance on a clear share of the disc.
    hittable_list fuzzy_world;
    fuzzy_world.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<metal>(color(0.8, 0.8, 0.8), 0.5)));
    ppm fuzzy_img = render_ppm(fuzzy_world, 16.0 / 9.0, 160, 20);
    int diff = 0, disc = 0;
    for (int j = 20; j < 70; j++)
        for (int i = 50; i < 110; i++) {
            auto a = metal_img.at(i, j), b = fuzzy_img.at(i, j);
            if (a[0] > 100) {  // inside/near the bright disc
                disc++;
                if (std::abs(a[0] - b[0]) > 10 ||
                    std::abs(a[1] - b[1]) > 10 ||
                    std::abs(a[2] - b[2]) > 10)
                    diff++;
            }
        }
    CHECK(disc > 100 && diff * 20 > disc);  // >5% differ

    // Dielectric centre is not its flat colour (light passes through).
    hittable_list glass_world;
    glass_world.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<dielectric>(1.5)));
    ppm glass_img = render_ppm(glass_world, 16.0 / 9.0, 160, 20);
    auto gcentre = glass_img.at(80, 45);
    CHECK(std::abs(gcentre[0] - centre[0]) > 20 ||
          std::abs(gcentre[1] - centre[1]) > 20);

    // Hollow glass (outer 1.5 + inner 1.00/1.50) renders completely.
    hittable_list hollow;
    hollow.add(make_shared<sphere>(point3(0, 0, -1), 0.5,
                                   make_shared<dielectric>(1.5)));
    hollow.add(make_shared<sphere>(point3(0, 0, -1), 0.4,
                                   make_shared<dielectric>(1.00 / 1.50)));
    ppm hollow_img = render_ppm(hollow, 16.0 / 9.0, 160, 20);
    CHECK(hollow_img.px.size() == (size_t)160 * 90);
}

static void test_antialiasing() {
    auto world = single_sphere_scene(color(1, 0, 0));
    ppm hard = render_ppm(world, 16.0 / 9.0, 160, 1);
    ppm soft = render_ppm(world, 16.0 / 9.0, 160, 100);
    int j = 45;
    // 1-sample row: a hard jump sky -> sphere in one pixel step.
    bool hard_jump = false;
    for (int i = 1; i < 160; i++) {
        if (std::abs(hard.at(i, j)[0] - hard.at(i - 1, j)[0]) > 50)
            hard_jump = true;
    }
    CHECK(hard_jump);
    // Many-sample row: at least one intermediate pixel between sky
    // red (~180) and sphere red (~255) beyond tolerance from both.
    bool intermediate = false;
    for (int i = 1; i < 160; i++) {
        int v = soft.at(i, j)[0];
        if (v > 190 && v < 245)
            intermediate = true;
    }
    CHECK(intermediate);
    // Flat interiors unchanged within tolerance, compared as patch
    // means: a 1-sample diffuse centre pixel is a single random
    // bounce and can be far from the averaged value on its own.
    auto hard_centre = patch_mean(hard, 80, 45);
    auto soft_centre = patch_mean(soft, 80, 45);
    auto hard_corner = patch_mean(hard, 5, 5);
    auto soft_corner = patch_mean(soft, 5, 5);
    for (int c = 0; c < 3; c++) {
        CHECK(std::abs(hard_centre[c] - soft_centre[c]) <= 20);
        CHECK(std::abs(hard_corner[c] - soft_corner[c]) <= 10);
    }
}

static void test_ground_no_shadow_no_speckle() {
    auto ground = make_shared<sphere>(
        point3(0, -100.5, -1), 100.0, make_shared<lambertian>(color(0.8, 0.8, 0.0)));
    hittable_list with_sphere;
    with_sphere.add(ground);
    with_sphere.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<lambertian>(color(1, 0, 0))));
    hittable_list ground_only;
    ground_only.add(ground);

    ppm with_img = render_ppm(with_sphere, 16.0 / 9.0, 160, 50);
    ppm bare_img = render_ppm(ground_only, 16.0 / 9.0, 160, 50);

    // No cast shadow: far ground pixels match the no-sphere render.
    for (auto [i, j] : std::vector<std::pair<int, int>>{{10, 80}, {150, 80},
                                                       {10, 89}, {150, 89}}) {
        for (int c = 0; c < 3; c++)
            CHECK(std::abs(with_img.at(i, j)[c] - bare_img.at(i, j)[c]) <= 25);
    }
    // No black speckles inside the sphere disc (<1% pure black).
    int black = 0, disc = 0;
    for (int jj = 20; jj < 70; jj++)
        for (int ii = 55; ii < 105; ii++) {
            auto p = with_img.at(ii, jj);
            if (p[0] > p[1] + 20) {  // reddish => sphere pixel
                disc++;
                if (p[0] == 0 && p[1] == 0 && p[2] == 0)
                    black++;
            }
        }
    CHECK(disc > 100 && black * 100 < disc);
}

static void test_depth_and_defocus() {
    // Two facing reflective surfaces recurse; must terminate with a
    // complete PPM (parse already guarantees no NaN/out-of-range).
    hittable_list world;
    world.add(make_shared<sphere>(
        point3(0, 0, -1), 0.5, make_shared<metal>(color(0.9, 0.9, 0.9), 0.0)));
    world.add(make_shared<sphere>(
        point3(0, -100.5, -1), 100.0,
        make_shared<lambertian>(color(0.8, 0.8, 0.0))));
    ppm img = render_ppm(world, 16.0 / 9.0, 120, 20, 50);
    CHECK(img.px.size() == (size_t)120 * 67);

    // Defocus: book defocus scene, same geometry, angle 0 vs 10.
    auto scene = []() {
        hittable_list w;
        w.add(make_shared<sphere>(point3(0, -100.5, -1), 100.0,
                                  make_shared<lambertian>(color(0.8, 0.8, 0.0))));
        w.add(make_shared<sphere>(point3(0, 0, -1.2), 0.5,
                                  make_shared<lambertian>(color(0.1, 0.2, 0.5))));
        w.add(make_shared<sphere>(point3(-1, 0, -1), 0.5,
                                  make_shared<dielectric>(1.5)));
        w.add(make_shared<sphere>(point3(-1, 0, -1), 0.4,
                                  make_shared<dielectric>(1.00 / 1.50)));
        w.add(make_shared<sphere>(point3(1, 0, -1), 0.5,
                                  make_shared<metal>(color(0.8, 0.6, 0.2), 0.0)));
        return w;
    };
    auto sharp = render_ppm(scene(), 16.0 / 9.0, 160, 50, 50, 20,
                            point3(-2, 2, 1), point3(0, 0, -1), 0, 3.4);
    auto blurred = render_ppm(scene(), 16.0 / 9.0, 160, 50, 50, 20,
                              point3(-2, 2, 1), point3(0, 0, -1), 10.0, 3.4);
    // Count intermediate pixels on the centre row: blurred render
    // must have strictly more edge transition pixels than sharp.
    auto count_intermediate = [](const ppm& p) {
        int n = 0, j = p.h / 2;
        for (int i = 1; i < p.w - 1; i++) {
            auto a = p.at(i - 1, j), b = p.at(i, j), c = p.at(i + 1, j);
            int d1 = std::abs(b[0] - a[0]) + std::abs(b[1] - a[1]) +
                     std::abs(b[2] - a[2]);
            int d2 = std::abs(c[0] - b[0]) + std::abs(c[1] - b[1]) +
                     std::abs(c[2] - b[2]);
            if (d1 > 10 && d2 > 10)
                n++;
        }
        return n;
    };
    int sharp_n = count_intermediate(sharp);
    int blur_n = count_intermediate(blurred);
    std::cerr << "defocus intermediate pixels: sharp=" << sharp_n
              << " blurred=" << blur_n << "\n";
    CHECK(blur_n > sharp_n);
}

int main() {
    test_vec3_ray_interval();
    test_sphere_hit();
    test_material_scatter();
    test_sky_and_single_sphere();
    test_nearest_metal_dielectric();
    test_antialiasing();
    test_ground_no_shadow_no_speckle();
    test_depth_and_defocus();

    if (failures) {
        std::cerr << failures << " check(s) FAILED\n";
        return 1;
    }
    std::cout << "all tests passed\n";
    return 0;
}
