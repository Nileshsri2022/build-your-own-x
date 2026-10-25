// Scratchapixel Introduction to Ray Tracing — own implementation.
// Follows the concepts in docs/SPEC.md and docs/DESIGN.md (chapters 1-3):
// backward ray tracing, spheres only, one primary ray per pixel centre,
// basic colour x brightness shading, hard shadow rays, recursive
// reflection / refraction with a Fresnel term, ASCII P3 PPM output.
// No tutorial code is copied here; structure and names are ours.
//
// Numbers the docs left "to verify before code", fixed here in code
// review and recorded (they are implementation choices, NOT claimed
// to be the tutorial's exact values):
//   Camera: eye (0,0,0), forward -z, image plane z = -1, plane extent
//     [-1,1] in y and [-aspect, aspect] in x (aspect = width/height).
//   MAX_DEPTH = 5 (SPEC's example value).
//   EPSILON = 1e-4 for self-intersection / shadow origins (DESIGN).
//   Glass refractive index = 1.5 (ordinary glass).
//   Fresnel: Schlick's approximation, R0 = ((1-ior)/(1+ior))^2.
//     This is the standard Fresnel approximation; the docs did not
//     state a verified chapter-3 formula, so Schlick is a stated
//     choice, not a tutorial quote.
//   Reflective material: reflectivity 0.8 (SPEC: "mix by reflectivity").
//   Light brightness = 1.0, white, single point light.
//   No Phong, no ambient term, no anti-aliasing, no textures, no meshes.

#ifndef SCRATCHAPIXEL_RAYTRACER_HPP
#define SCRATCHAPIXEL_RAYTRACER_HPP

#include <cmath>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace raytracer {

constexpr double kEpsilon = 1e-4;
constexpr int kMaxDepth = 5;
constexpr double kGlassIor = 1.5;
constexpr double kReflectivity = 0.8;

struct Vec3 {
    double x = 0, y = 0, z = 0;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator*(const Vec3& o) const { return {x * o.x, y * o.y, z * o.z}; }
    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    double length() const { return std::sqrt(dot(*this)); }
    // Zero vector normalizes to (0,0,0), never NaN (TEST_PLAN decision).
    Vec3 normalized() const {
        double len = length();
        if (len == 0) return {0, 0, 0};
        return *this * (1.0 / len);
    }
};

using Color = Vec3;  // channels in [0,1], clamped at output / trace return.

inline Color clamped(Color c) {
    auto clamp1 = [](double v) { return v < 0 ? 0 : (v > 1 ? 1 : v); };
    return {clamp1(c.x), clamp1(c.y), clamp1(c.z)};
}

struct Ray {
    Vec3 origin;
    Vec3 direction;  // caller normalizes; intersect() tolerates any length.
    Vec3 pointAt(double t) const { return origin + direction * t; }
};

enum class Material { Opaque, Reflective, Glass, Light };

struct Sphere {
    Vec3 center;
    double radius = 1;
    Color color{0, 0, 0};
    Material material = Material::Opaque;
};

struct Hit {
    double t = 0;
    Vec3 point;
    Vec3 normal;  // outward, unit length
    const Sphere* sphere = nullptr;
};

// Sphere intersection per SPEC section 3: a = d.d, b = 2*d.(o-C),
// c = (o-C).(o-C) - r^2, discriminant b^2 - 4*a*c. Nearest positive
// root with t > kEpsilon wins; no hit otherwise.
inline std::optional<Hit> intersectSphere(const Ray& ray, const Sphere& s) {
    Vec3 oc = ray.origin - s.center;
    double a = ray.direction.dot(ray.direction);
    if (a == 0) return std::nullopt;
    double b = 2.0 * ray.direction.dot(oc);
    double c = oc.dot(oc) - s.radius * s.radius;
    double disc = b * b - 4 * a * c;
    if (disc < 0) return std::nullopt;
    double sq = std::sqrt(disc);
    double t = (-b - sq) / (2 * a);
    if (t <= kEpsilon) {
        t = (-b + sq) / (2 * a);  // ray started inside the sphere: exit hit
        if (t <= kEpsilon) return std::nullopt;
    }
    Vec3 point = ray.pointAt(t);
    Vec3 normal = (point - s.center) * (1.0 / s.radius);
    return Hit{t, point, normal, &s};
}

struct Light {
    Vec3 position;
    double brightness = 1.0;
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    Color background{0, 0, 0};

    std::optional<Hit> nearestHit(const Ray& ray) const {
        std::optional<Hit> best;
        for (const auto& s : spheres) {
            auto h = intersectSphere(ray, s);
            if (h && (!best || h->t < best->t)) best = h;
        }
        return best;
    }

    // Shadow test per SPEC: any (non-light) sphere hit at t < distance
    // to the light blocks that light. Light-marker spheres are skipped:
    // otherwise the marker at the light position would block its own light.
    // The hit sphere itself is also skipped: a shadow ray leaving a
    // front point must not count re-entering its own sphere as a
    // blocker (with the light in front of the scene, as in
    // defaultScene, this only matters for grazing points).
    bool blocked(const Vec3& point, const Vec3& normal, const Light& light,
                 const Sphere* exclude = nullptr) const {
        Vec3 toLight = light.position - point;
        double dist = toLight.length();
        if (dist == 0) return false;
        Ray shadow{point + normal * kEpsilon, toLight * (1.0 / dist)};
        for (const auto& s : spheres) {
            if (&s == exclude) continue;
            if (s.material == Material::Light) continue;
            auto h = intersectSphere(shadow, s);
            if (h && h->t < dist) return true;
        }
        return false;
    }
};

// Default scene (ours, per SPEC section 5): centre red opaque sphere,
// green reflective sphere above-right of it (between light and red's
// right side, so it casts a real shadow onto that side only — the
// centre pixel stays lit), blue glass sphere to the right, and a
// small white light-marker sphere at the point-light position.
// The light sits in front of the spheres (z = -3, camera side), so
// front points face it; a light at the spheres' own depth would put
// front points in their own sphere's shadow.
inline Scene defaultScene() {
    Scene scene;
    scene.spheres = {
        {{0, 0, -5}, 1.0, {1, 0, 0}, Material::Opaque},        // centre red
        {{0.9, 1.2, -3.8}, 0.8, {0, 1, 0}, Material::Reflective},  // green
        {{2.1, 0, -5}, 1.0, {0, 0, 1}, Material::Glass},        // right blue
        {{0, 3, -3}, 0.5, {1, 1, 1}, Material::Light},          // light marker
    };
    scene.lights = {{{0, 3, -3}, 1.0}};
    scene.background = {0, 0, 0};
    return scene;
}

inline Vec3 reflectDir(const Vec3& d, const Vec3& n) {
    return d - n * (2.0 * d.dot(n));
}

// Snell's law. Returns false on total internal reflection.
inline bool refractDir(const Vec3& d, const Vec3& n, double eta, Vec3& out) {
    double cosi = -d.dot(n);
    double k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    if (k < 0) return false;
    out = d * eta + n * (eta * cosi - std::sqrt(k));
    return true;
}

// Schlick's Fresnel approximation. cosTheta is the cosine between the
// (unit) incoming direction reversed and the surface normal, in [0,1].
inline double fresnelSchlick(double cosTheta, double ior) {
    double r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;
    double m = 1.0 - cosTheta;
    return r0 + (1.0 - r0) * m * m * m * m * m;
}

inline Color trace(const Scene& scene, const Ray& ray, int depth);

inline Color trace(const Scene& scene, const Ray& ray, int depth) {
    auto hit = scene.nearestHit(ray);
    if (!hit) return scene.background;
    const Sphere& s = *hit->sphere;
    if (s.material == Material::Light) return s.color;

    Color color{0, 0, 0};
    for (const auto& light : scene.lights) {
        if (scene.blocked(hit->point, hit->normal, light, &s)) continue;
        color = color + s.color * light.brightness;
    }

    if (depth >= kMaxDepth) return clamped(color);
    Vec3 d = ray.direction.normalized();
    if (s.material == Material::Reflective) {
        Ray refl{hit->point + hit->normal * kEpsilon, reflectDir(d, hit->normal)};
        Color reflected = trace(scene, refl, depth + 1);
        color = color * (1.0 - kReflectivity) + reflected * kReflectivity;
    } else if (s.material == Material::Glass) {
        bool outside = d.dot(hit->normal) < 0;
        Vec3 n = outside ? hit->normal : -hit->normal;
        double eta = outside ? 1.0 / kGlassIor : kGlassIor;
        double cosi = -d.dot(n);
        Vec3 refracted{0, 0, 0};
        double kr;
        if (!refractDir(d, n, eta, refracted)) {
            kr = 1.0;  // total internal reflection
        } else {
            kr = fresnelSchlick(cosi, kGlassIor);
        }
        Ray refl{hit->point + hit->normal * kEpsilon, reflectDir(d, hit->normal)};
        Ray refr{hit->point - hit->normal * kEpsilon, refracted};
        Color reflected = trace(scene, refl, depth + 1);
        Color refractedColor = kr >= 1.0 ? Color{0, 0, 0} : trace(scene, refr, depth + 1);
        color = reflected * kr + refractedColor * (1.0 - kr);
    }
    return clamped(color);
}

// Primary ray through the centre of pixel (x, y), y = 0 at the top.
inline Ray primaryRay(int x, int y, int width, int height) {
    double aspect = static_cast<double>(width) / static_cast<double>(height);
    double px = (2.0 * (x + 0.5) / width - 1.0) * aspect;
    double py = 1.0 - 2.0 * (y + 0.5) / height;
    Vec3 dir = Vec3{px, py, -1.0}.normalized();
    return Ray{{0, 0, 0}, dir};
}

struct Image {
    int width = 0, height = 0;
    std::vector<Color> pixels;  // row-major, y = 0 at the top
    Color at(int x, int y) const { return pixels[y * width + x]; }
};

// PPM writer (SPEC section 6): ASCII P3, header P3 / width height / 255,
// then one decimal RGB triple per line, channels clamped and rounded.
inline int toByte(double v) {
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    return static_cast<int>(std::round(v * 255));
}

inline bool writePpm(const std::string& path, const Image& img) {
    std::ofstream out(path);
    if (!out) return false;
    out << "P3\n" << img.width << ' ' << img.height << "\n255\n";
    for (const auto& px : img.pixels)
        out << toByte(px.x) << ' ' << toByte(px.y) << ' ' << toByte(px.z) << '\n';
    out.flush();
    return static_cast<bool>(out);
}

inline Image render(const Scene& scene, int width, int height) {
    Image img{width, height, std::vector<Color>(static_cast<size_t>(width) * height)};
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            img.pixels[static_cast<size_t>(y) * width + x] =
                trace(scene, primaryRay(x, y, width, height), 0);
    return img;
}

}  // namespace raytracer

#endif
