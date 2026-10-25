// Ch. 2 — Monte Carlo (scoped to what Ch. 13 needs) + the minimum
// sampler the path tracer requires (SPEC §3: Ch. 8 is NOT in scope in
// full; this is a seeded uniform RNG plus inversion-method transforms).
#pragma once
#include <random>
#include <cmath>
#include "geometry.h"

namespace pbr {

struct Sampler {
    std::mt19937_64 rng;
    explicit Sampler(uint64_t seed) : rng(seed) {}
    double uniform() { return std::uniform_real_distribution<double>(0, 1)(rng); }
    // Inversion method: uniform -> exponential with rate lambda.
    static double exponential(double u, double lambda) { return -std::log(1 - u) / lambda; }
    // Uniform point on the unit sphere (inversion of the cosine CDF).
    Vec3 uniformSphere(double u1, double u2) const {
        double z = 1 - 2 * u1, r = std::sqrt(std::max(0.0, 1 - z * z)), phi = 2 * M_PI * u2;
        return {r * std::cos(phi), r * std::sin(phi), z};
    }
    // Cosine-weighted hemisphere sample around normal n.
    Vec3 cosineHemisphere(const Vec3& n, double u1, double u2) const {
        double r = std::sqrt(u1), phi = 2 * M_PI * u2;
        Vec3 local{r * std::cos(phi), r * std::sin(phi), std::sqrt(std::max(0.0, 1 - u1))};
        Vec3 t = std::abs(n.z) < 0.999 ? normalize(cross(n, {0, 0, 1})) : normalize(cross(n, {1, 0, 0}));
        Vec3 b = cross(n, t);
        return normalize(t * local.x + b * local.y + n * local.z);
    }
};

}  // namespace pbr
