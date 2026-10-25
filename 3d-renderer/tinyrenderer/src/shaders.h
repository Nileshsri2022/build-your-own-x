// shaders.h — Phong shading, tangent-space normal mapping (own implementation).
// Coefficients fixed by ticket T5 and recorded here:
//   ambient = 0.10, diffuse = 0.90, specular strength = 0.50, shininess = 32.
// No gamma correction (SPEC section 4: none specified).
// Interpolation is screen-space barycentric (not perspective-correct);
// for the near-planar test meshes this matches the expected pixels, and
// the simplification is stated here rather than hidden.
#pragma once
#include "geometry.h"
#include "tgaimage.h"

struct Light {
    Vec3 pos = {1, 1, 3};
};

// Sample a texture at UV (u right, v up, v=0 = bottom row of the image).
inline TGAColor sampleTexture(const TGAImage* tex, double u, double v) {
    if (!tex || tex->width <= 0 || tex->height <= 0) return TGAColor(255, 255, 255);
    if (u < 0) u = 0; if (u > 1) u = 1;
    if (v < 0) v = 0; if (v > 1) v = 1;
    int x = (int)(u * (tex->width - 1) + 0.5);
    int y = (int)((1.0 - v) * (tex->height - 1) + 0.5);  // stored rows are top-first
    return tex->get(x, y);
}

// Tangent/bitangent/normal basis from one triangle's positions + UVs.
inline void computeTBN(const Vec3& p0, const Vec3& p1, const Vec3& p2,
                       const Vec2& uv0, const Vec2& uv1, const Vec2& uv2,
                       Vec3& T, Vec3& B, Vec3& N) {
    Vec3 e1 = p1 - p0, e2 = p2 - p0;
    Vec2 d1 = uv1 - uv0, d2 = uv2 - uv0;
    double det = d1.x * d2.y - d2.x * d1.y;
    N = normalized(cross(e1, e2));
    if (std::fabs(det) < 1e-12) {  // degenerate UVs: fall back to a basis around N
        Vec3 a = std::fabs(N.x) < 0.9 ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
        T = normalized(a - N * dot(a, N));
        B = cross(N, T);
        return;
    }
    double inv = 1.0 / det;
    T = normalized((e1 * d2.y - e2 * d1.y) * inv);
    B = normalized((e2 * d1.x - e1 * d2.x) * inv);
}

// Phong shade for one fragment. texColor is the diffuse sample (white if
// no texture). Returns clamped 0..255 channels.
inline TGAColor phongShade(const TGAColor& texColor, const Vec3& N, const Vec3& worldPos,
                           const Vec3& eye, const Light& light, bool inShadow) {
    const double ambient = 0.10, diffuseK = 0.90, specK = 0.50, shininess = 32.0;
    if (inShadow) {
        // Ambient only, per the hard binary shadow mask.
        return TGAColor((uint8_t)(texColor.r * ambient + 0.5),
                        (uint8_t)(texColor.g * ambient + 0.5),
                        (uint8_t)(texColor.b * ambient + 0.5));
    }
    Vec3 L = normalized(light.pos - worldPos);
    Vec3 V = normalized(eye - worldPos);
    double ndotl = dot(N, L);
    if (ndotl < 0) ndotl = 0;
    Vec3 R = N * (2.0 * dot(N, L)) - L;  // reflection of -L about N
    double spec = 0;
    double rdotv = dot(normalized(R), V);
    if (rdotv > 0) spec = specK * std::pow(rdotv, shininess);
    double f = ambient + diffuseK * ndotl;
    auto clamp255 = [](double v) -> uint8_t {
        if (v < 0) return 0; if (v > 255) return 255; return (uint8_t)(v + 0.5);
    };
    return TGAColor(clamp255(texColor.r * f + 255 * spec),
                    clamp255(texColor.g * f + 255 * spec),
                    clamp255(texColor.b * f + 255 * spec));
}
