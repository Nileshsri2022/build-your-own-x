// Ch. 13 — Simple path tracer (the target renderer).
// CODE-TIME DECISIONS (SPEC §4, Ch. 13 values were NOT verified
// against chapter text — chapter bodies were not read):
//  - max depth 5, fixed (no Russian roulette in this build);
//  - miss returns a constant background colour;
//  - direct light comes from point lights with shadow rays. This
//    is required by TEST_PLAN's "diffuse sphere under a light"
//    test; it is NOT Ch. 12 in full (no area/infinite lights, no
//    light-sampling PDFs). Delta BSDFs get no direct term, to
//    avoid double-counting their reflected/transmitted paths.
#pragma once
#include "primitives.h"
#include "montecarlo.h"

namespace pbr {

struct PointLight { Vec3 pos; Spectrum intensity; };  // intensity = power / (4*pi) style scalar colour

class PathTracer {
public:
    PathTracer(const BVH& bvh, std::vector<PointLight> lights, int maxDepth = 5)
        : bvh_(bvh), lights_(std::move(lights)), maxDepth_(maxDepth) {}
    Spectrum background{0.05, 0.07, 0.10};
    Spectrum Li(const Ray& ray, Sampler& s, int depth = 0) const {
        SceneHit sh;
        if (!bvh_.intersect(ray, 1e-4, sh)) return background;
        const BSDF& bsdf = *sh.prim->bsdf;
        Vec3 wo = -ray.dir;
        Vec3 n = sh.hit.normal.asVec();
        Spectrum L{};
        // Direct lighting (non-delta BSDFs only).
        for (const auto& light : lights_) {
            Vec3 toLight = light.pos - sh.hit.point;
            double dist2 = toLight.length2(), dist = std::sqrt(dist2);
            Vec3 wi = toLight / dist;
            if (dot(wi, n) <= 0) continue;
            Spectrum fv = bsdf.f(wo, wi, n);
            if (fv == Spectrum{}) continue;
            Ray shadow{sh.hit.point + n * 1e-4, wi, dist - 2e-4};
            SceneHit occ;
            if (bvh_.intersect(shadow, 1e-4, occ)) continue;  // in shadow
            L += fv * light.intensity * dot(wi, n) / dist2;
        }
        if (depth >= maxDepth_) return L;
        BSDFSample bs = bsdf.sample(wo, n, s);
        if (!bs.valid) return L;
        // Offset along the sampled direction's side of the surface.
        Ray next{sh.hit.point + (dot(bs.wi, n) >= 0 ? n : -n) * 1e-4, bs.wi};
        L += bs.throughput * Li(next, s, depth + 1);
        return L;
    }
private:
    const BVH& bvh_;
    std::vector<PointLight> lights_;
    int maxDepth_;
};

}  // namespace pbr
