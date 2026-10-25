// Ch. 4 — Radiometry / colour.
// CODE-TIME DECISION (SPEC §2, Ch. 4): this build uses an RGB
// approximation, not full spectral rendering. That is our decision,
// not a claim about what the book's simple tracer assumes; the
// chapter bodies were not read. Spectrum is therefore an RGB triple.
#pragma once
#include "geometry.h"

namespace pbr {
using Spectrum = Vec3;  // (r, g, b) radiance / reflectance
inline Spectrum clamp01(const Spectrum& s) {
    auto c = [](double v) { return v < 0 ? 0 : (v > 1 ? 1 : v); };
    return {c(s.x), c(s.y), c(s.z)};
}
}  // namespace pbr
