// Assert-based unit tests for the scoped PBR build (TEST_PLAN §1).
// Expected values are computed by hand here, not by re-calling the
// production code under test (except BVH-vs-brute-force, which is
// precisely the property being tested).
#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
#include "../src/geometry.h"
#include "../src/montecarlo.h"
#include "../src/camera.h"
#include "../src/film.h"
#include "../src/shapes.h"
#include "../src/primitives.h"
#include "../src/bsdf.h"
#include "../src/integrator.h"

using namespace pbr;
static int checks = 0;
#define CHECK(c) do { assert(c); ++checks; } while (0)
static bool near(double a, double b, double tol = 1e-6) { return std::abs(a - b) <= tol; }
static bool nearV(const Vec3& a, const Vec3& b, double tol = 1e-6) {
    return near(a.x, b.x, tol) && near(a.y, b.y, tol) && near(a.z, b.z, tol);
}

int main() {
    // --- Geometry (Ch. 3) ---
    CHECK(nearV(Vec3{1,2,3} + Vec3{4,5,6}, Vec3{5,7,9}));
    CHECK(near(dot(Vec3{1,0,0}, Vec3{0,1,0}), 0));
    CHECK(nearV(cross(Vec3{1,0,0}, Vec3{0,1,0}), Vec3{0,0,1}));
    CHECK(near(Vec3{3,4,0}.length(), 5));
    CHECK(near(normalize(Vec3{0,0,5}).z, 1));
    { Vec3 z = normalize(Vec3{0,0,0}); CHECK(z == Vec3{} && !std::isnan(z.x)); }  // zero -> zero, not NaN
    { // Normals transform differently from points: non-uniform scale.
      Transform tr = Transform::scale(1, 2, 1);
      Vec3 v = tr.vector(Vec3{1, 1, 0});          // (1,2,0)
      Normal n = tr.normal(Normal{1, 1, 0});      // inverse-transpose: (1,0.5,0) normalised
      CHECK(nearV(v, Vec3{1, 2, 0}));
      CHECK(near(n.x, 1 / std::sqrt(1.25), 1e-9) && near(n.y, 0.5 / std::sqrt(1.25), 1e-9));
    }
    { Bounds b; b.grow(Vec3{-1,-1,-1}); b.grow(Vec3{1,1,1});
      CHECK(b.hit(Ray{Vec3{0,0,0}, Vec3{1,0,0}}, 0, 10));       // starts inside
      CHECK(!b.hit(Ray{Vec3{5,5,5}, Vec3{1,0,0}}, 0, 10));      // misses
      CHECK(b.hit(Ray{Vec3{-5,0,0}, Vec3{1,0,0}}, 0, 10));
    }
    // --- Monte Carlo (Ch. 2) ---
    { // Inversion: exponential mean 1/lambda over many samples.
      Sampler s(123); double sum = 0; int N = 20000;
      for (int i = 0; i < N; ++i) sum += Sampler::exponential(s.uniform(), 2.0);
      CHECK(std::abs(sum / N - 0.5) < 0.02);
      // Cosine-weighted hemisphere: E[cos] = 2/3.
      double csum = 0;
      for (int i = 0; i < N; ++i) {
          Vec3 d = s.cosineHemisphere(Vec3{0,1,0}, s.uniform(), s.uniform());
          csum += d.y;
      }
      CHECK(std::abs(csum / N - 2.0/3) < 0.02);
    }
    // --- Shapes (Ch. 6) ---
    Sphere sph(Vec3{0,0,0}, 1);
    { Hit h; CHECK(sph.intersect(Ray{Vec3{0,0,-5}, Vec3{0,0,1}}, 0, h) && near(h.t, 4)); }
    { Hit h; CHECK(!sph.intersect(Ray{Vec3{2,0,-5}, Vec3{0,0,1}}, 0, h)); }
    { Hit h; CHECK(sph.intersect(Ray{Vec3{1,0,-5}, Vec3{0,0,1}}, 0, h) && near(h.t, 5, 1e-6)); }  // tangent
    { Hit h; CHECK(sph.intersect(Ray{Vec3{0,0,0}, Vec3{0,0,1}}, 0, h) && near(h.t, 1)); }          // from inside
    { Hit h; CHECK(!sph.intersect(Ray{Vec3{0,0,5}, Vec3{0,0,1}}, 0, h)); }                        // pointing away
    Cylinder cyl(Vec3{0,0,0}, 1, 2);
    { Hit h; CHECK(cyl.intersect(Ray{Vec3{2,0,0}, Vec3{-1,0,0}}, 0, h) && near(h.t, 1)); }
    { Hit h; CHECK(cyl.intersect(Ray{Vec3{0,3,0}, Vec3{0,-1,0}}, 0, h) && near(h.t, 2)); }  // top cap
    { Hit h; CHECK(!cyl.intersect(Ray{Vec3{0,3,0}, Vec3{0,1,0}}, 0, h)); }
    Disk disk(Vec3{0,0,0}, Vec3{0,1,0}, 1);
    { Hit h; CHECK(disk.intersect(Ray{Vec3{0,5,0}, Vec3{0,-1,0}}, 0, h) && near(h.t, 5)); }
    { Hit h; CHECK(!disk.intersect(Ray{Vec3{2,5,0}, Vec3{0,-1,0}}, 0, h)); }
    { // Shared edge: two triangles forming a quad; a ray exactly on the
      // shared diagonal must hit (no crack).
      TriangleMesh mesh({Triangle{{0,0,0},{1,0,0},{1,1,0}}, Triangle{{0,0,0},{1,1,0},{0,1,0}}});
      Hit h; CHECK(mesh.intersect(Ray{Vec3{0.5,0.5,-1}, Vec3{0,0,1}}, 0, h) && near(h.t, 1));
      Hit h2; CHECK(!mesh.intersect(Ray{Vec3{2,2,-1}, Vec3{0,0,1}}, 0, h2));  // through a gap
    }
    // --- BVH vs brute force (Ch. 7) + timing ---
    std::vector<Primitive> prims;
    for (int i = 0; i < 200; ++i)
        prims.push_back(Primitive{std::make_shared<Sphere>(Vec3{(double)((i % 10) - 5), (double)((i / 10) % 10 - 5), -i * 0.1}, 0.4),
                                  std::make_shared<DiffuseBSDF>(Spectrum{0.5,0.5,0.5})});
    BVH bvh(prims);
    { Sampler s(7); int agree = 0, total = 500;
      for (int i = 0; i < total; ++i) {
          Ray r{Vec3{s.uniform()*10-5, s.uniform()*10-5, 5}, normalize(Vec3{s.uniform()-0.5, s.uniform()-0.5, -1})};
          SceneHit a, b; bool ha = bvh.intersect(r, 0, a), hb = bruteIntersect(prims, r, 0, b);
          if (ha == hb && (!ha || near(a.hit.t, b.hit.t, 1e-6))) ++agree;
      }
      CHECK(agree == total);
    }
    { // Timing: 2000 spheres, 2000 rays — BVH must be faster than brute force.
      std::vector<Primitive> many;
      for (int i = 0; i < 2000; ++i)
          many.push_back(Primitive{std::make_shared<Sphere>(Vec3{(double)((i % 40) - 20), (double)((i / 40) % 40 - 20), (double)(-(i % 7))}, 0.4),
                                    std::make_shared<DiffuseBSDF>(Spectrum{0.5,0.5,0.5})});
      BVH big(many);
      std::vector<Ray> rays;
      for (int i = 0; i < 2000; ++i) rays.push_back(Ray{Vec3{(double)((i % 40) - 20), (double)((i / 40) - 20), 5}, Vec3{0,0,-1}});
      auto t0 = std::chrono::steady_clock::now();
      for (auto& r : rays) { SceneHit h; big.intersect(r, 0, h); }
      auto t1 = std::chrono::steady_clock::now();
      for (auto& r : rays) { SceneHit h; bruteIntersect(many, r, 0, h); }
      auto t2 = std::chrono::steady_clock::now();
      double tb = std::chrono::duration<double>(t1 - t0).count();
      double tf = std::chrono::duration<double>(t2 - t1).count();
      std::cout << "BVH time " << tb << "s, brute time " << tf << "s\n";
      CHECK(tb < tf);
    }
    // --- Reflection models (Ch. 9) ---
    DiffuseBSDF diff(Spectrum{0.6, 0.3, 0.1});
    CHECK(nearV(diff.f(Vec3{0,0,1}, Vec3{0,0,1}, Vec3{0,0,1}), Spectrum{0.6/M_PI, 0.3/M_PI, 0.1/M_PI}, 1e-9));
    { Sampler s(3); auto bs = diff.sample(Vec3{0,0,1}, Vec3{0,0,1}, s);
      CHECK(bs.valid && bs.wi.z > 0 && nearV(bs.throughput, Spectrum{0.6,0.3,0.1})); }
    MirrorBSDF mir;
    { Sampler s(3); auto bs = mir.sample(Vec3{0,0,1}, Vec3{0,0,1}, s);
      CHECK(bs.valid && near(bs.wi.z, 1)); }  // wo (0,0,1) means incoming (0,0,-1) reflects to (0,0,1)
    { // Snell's law: refract (0,0,-1)-ish at 30 degrees through eta 1.5.
      Vec3 d = normalize(Vec3{0.5, 0, -std::sqrt(0.75)}), out;
      CHECK(refract(d, Vec3{0,0,1}, 1.0/1.5, out));
      CHECK(near(out.x, 0.5/1.5, 1e-6));  // sin(theta_t) = sin(theta_i)/eta = 0.5/1.5
      // Total internal reflection past the critical angle (eta 1.5, sin > 1/1.5).
      Vec3 d3 = normalize(Vec3{std::sqrt(1 - 0.25), 0, 0.5}), out2;  // sin=0.866 > 1/1.5, exiting
      CHECK(!refract(d3, Vec3{0,0,-1}, 1.5, out2));
      double fr = fresnelDielectric(1.0, 1.5);
      CHECK(fr > 0 && fr < 1);
    }
    ConductorBSDF cond(Spectrum{0.9, 0.7, 0.4});
    { Sampler s(3); auto bs = cond.sample(Vec3{0,0,1}, Vec3{0,0,1}, s);
      CHECK(bs.valid && bs.wi.z > 0 && bs.throughput.x > bs.throughput.z); }  // tinted, no transmission direction
    { // Microfacet: roughness spreads the reflection.
      MicrofacetBSDF smooth(Spectrum{1,1,1}, 0.05), rough(Spectrum{1,1,1}, 0.8);
      Sampler s(9); double spreadS = 0, spreadR = 0; int N = 4000;
      for (int i = 0; i < N; ++i) {
          auto a = smooth.sample(Vec3{0,0,1}, Vec3{0,0,1}, s);
          auto b = rough.sample(Vec3{0,0,1}, Vec3{0,0,1}, s);
          if (a.valid) spreadS += std::acos(std::min(1.0, a.wi.z));
          if (b.valid) spreadR += std::acos(std::min(1.0, b.wi.z));
      }
      CHECK(spreadR > spreadS * 2);
    }
    // --- Cameras and film (Ch. 5) ---
    ProjectiveCamera cam(Vec3{0,0,0}, Vec3{0,0,-1}, Vec3{0,1,0}, 90, 1);
    CHECK(nearV(cam.generateRay(0.5, 0.5).dir, Vec3{0,0,-1}, 1e-9));
    CHECK(nearV(cam.generateRay(1, 1).dir, normalize(Vec3{1,1,-1}), 1e-9));
    CHECK(near(cam.fovDeg(), 90));
    { Film film(2, 1); film.addSample(0, 0, Spectrum{0.2,0.2,0.2}); film.addSample(0, 0, Spectrum{0.4,0.4,0.4});
      CHECK(nearV(film.pixel(0, 0), Spectrum{0.3,0.3,0.3})); CHECK(film.width() == 2 && film.height() == 1); }
    // --- Path tracer (Ch. 13) ---
    { // Empty scene -> background only.
      BVH empty(std::vector<Primitive>{});
      PathTracer pt(empty, {});
      Sampler s(1);
      CHECK(nearV(pt.Li(Ray{Vec3{0,0,0}, Vec3{0,0,-1}}, s), pt.background));
    }
    { // Diffuse sphere under a point light: centre brighter than limb.
      std::vector<Primitive> sc{Primitive{std::make_shared<Sphere>(Vec3{0,0,0}, 1),
                                          std::make_shared<DiffuseBSDF>(Spectrum{0.8,0.8,0.8})}};
      BVH b(sc);
      PathTracer pt(b, {PointLight{Vec3{0, 0, 5}, Spectrum{50, 50, 50}}});
      Sampler s(1);
      Spectrum centre = pt.Li(Ray{Vec3{0,0,5}, Vec3{0,0,-1}}, s);
      Spectrum limb = pt.Li(Ray{Vec3{0.9,0,5}, normalize(Vec3{-0.18,0,-1})}, s);
      CHECK(centre.x > limb.x + 0.02);
    }
    { // Mirror sphere reflects another object (red sphere behind camera side).
      std::vector<Primitive> sc{
          Primitive{std::make_shared<Sphere>(Vec3{0,0,0}, 1), std::make_shared<MirrorBSDF>()},
          Primitive{std::make_shared<Sphere>(Vec3{0,0,3}, 1), std::make_shared<DiffuseBSDF>(Spectrum{0.9,0.1,0.1})}};
      BVH b(sc);
      PathTracer pt(b, {PointLight{Vec3{0, 3, 5}, Spectrum{50, 50, 50}}});
      Sampler s(1);
      Spectrum c = pt.Li(Ray{Vec3{0,0,5}, Vec3{0,0,-1}}, s);
      CHECK(c.x > c.y + 0.01);  // reflected red, not mirror's own (white) colour
    }
    { // Two facing mirrors terminate (depth cap) — just must return.
      std::vector<Primitive> sc{
          Primitive{std::make_shared<Sphere>(Vec3{0,0,-1}, 1), std::make_shared<MirrorBSDF>()},
          Primitive{std::make_shared<Sphere>(Vec3{0,0,1}, 1), std::make_shared<MirrorBSDF>()}};
      BVH b(sc);
      PathTracer pt(b, {});
      Sampler s(1);
      Spectrum c = pt.Li(Ray{Vec3{0,0,0}, Vec3{0,0,1}}, s);
      CHECK(!std::isnan(c.x));
    }
    std::cout << "ALL TESTS PASSED (" << checks << " checks)\n";
    return 0;
}
