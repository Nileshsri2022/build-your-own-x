// Ch. 9 — Reflection models. Interface: sample / evaluate / PDF.
// Our own implementation from the concepts (diffuse, specular,
// conductor, dielectric, microfacet roughness, rough dielectric).
// Excluded, with no code here: measured BSDFs, hair scattering.
// Fresnel terms use Schlick's approximation (a standard closed form,
// stated here so it is not mistaken for a quote from the book).
#pragma once
#include <cmath>
#include "geometry.h"
#include "radiometry.h"
#include "montecarlo.h"

namespace pbr {

inline double fresnelSchlick(double cosI, double f0) {
    double m = 1 - cosI;
    return f0 + (1 - f0) * m * m * m * m * m;
}
// Dielectric Fresnel (exact, from Snell's law + Fresnel equations).
inline double fresnelDielectric(double cosI, double eta) {
    cosI = std::min(1.0, std::max(-1.0, cosI));
    bool entering = cosI > 0;
    double ei = entering ? 1.0 : eta, et = entering ? eta : 1.0;
    double sinT2 = (ei / et) * (ei / et) * (1 - cosI * cosI);
    if (sinT2 >= 1) return 1.0;  // total internal reflection
    double cosT = std::sqrt(1 - sinT2);
    double rPar = ((et * cosI) - (ei * cosT)) / ((et * cosI) + (ei * cosT));
    double rPer = ((ei * cosI) - (et * cosT)) / ((ei * cosI) + (et * cosT));
    return 0.5 * (rPar * rPar + rPer * rPer);
}
inline double ggxD(double cosH, double alpha) {
    double a2 = alpha * alpha, c2 = cosH * cosH;
    double d = c2 * (a2 - 1) + 1;
    return a2 / (M_PI * d * d);
}
inline double ggxG1(double cosV, double alpha) {
    double a2 = alpha * alpha;
    return 2 * cosV / (cosV + std::sqrt(a2 + (1 - a2) * cosV * cosV));
}

struct BSDFSample {
    Vec3 wi; Spectrum throughput; double pdf = 0; bool isDelta = false; bool valid = false;
};

class BSDF {
public:
    virtual ~BSDF() = default;
    virtual Spectrum f(const Vec3& wo, const Vec3& wi, const Vec3& n) const = 0;
    virtual double pdf(const Vec3& wo, const Vec3& wi, const Vec3& n) const = 0;
    virtual BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler& s) const = 0;
protected:
    static Vec3 face(const Vec3& n, const Vec3& wo) { return dot(n, wo) >= 0 ? n : -n; }
};

class DiffuseBSDF : public BSDF {
public:
    explicit DiffuseBSDF(Spectrum albedo) : albedo_(albedo) {}
    Spectrum f(const Vec3&, const Vec3& wi, const Vec3& n) const override {
        return dot(wi, n) > 0 ? albedo_ * (1 / M_PI) : Spectrum{};
    }
    double pdf(const Vec3&, const Vec3& wi, const Vec3& n) const override {
        double c = dot(wi, n); return c > 0 ? c / M_PI : 0;
    }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler& s) const override {
        Vec3 nn = face(n, wo);
        Vec3 wi = s.cosineHemisphere(nn, s.uniform(), s.uniform());
        double c = dot(wi, nn);
        return {wi, albedo_, c / M_PI, false, true};
    }
private:
    Spectrum albedo_;
};

class MirrorBSDF : public BSDF {
public:
    explicit MirrorBSDF(Spectrum tint = {1, 1, 1}) : tint_(tint) {}
    Spectrum f(const Vec3&, const Vec3&, const Vec3&) const override { return {}; }
    double pdf(const Vec3&, const Vec3&, const Vec3&) const override { return 0; }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler&) const override {
        Vec3 nn = face(n, wo);
        return {reflect(-wo, nn), tint_, 1, true, true};
    }
private:
    Spectrum tint_;
};

// Conductor: tinted mirror, no transmission. F0 is approximated from
// a scalar (eta, k) pair via the standard normal-incidence formula,
// per channel using the given tint as the F0 colour — an approximation,
// recorded here, not a book quote.
class ConductorBSDF : public BSDF {
public:
    ConductorBSDF(Spectrum tint, double eta = 0.37, double k = 2.82) : tint_(tint), eta_(eta), k_(k) {}
    Spectrum f(const Vec3&, const Vec3&, const Vec3&) const override { return {}; }
    double pdf(const Vec3&, const Vec3&, const Vec3&) const override { return 0; }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler&) const override {
        Vec3 nn = face(n, wo);
        double cosI = std::max(0.0, dot(wo, nn));
        double f0 = ((eta_ - 1) * (eta_ - 1) + k_ * k_) / ((eta_ + 1) * (eta_ + 1) + k_ * k_);
        double fr = fresnelSchlick(cosI, f0);
        return {reflect(-wo, nn), tint_ * fr, 1, true, true};
    }
private:
    Spectrum tint_; double eta_, k_;
};

class DielectricBSDF : public BSDF {
public:
    explicit DielectricBSDF(double eta) : eta_(eta) {}
    Spectrum f(const Vec3&, const Vec3&, const Vec3&) const override { return {}; }
    double pdf(const Vec3&, const Vec3&, const Vec3&) const override { return 0; }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler& s) const override {
        bool entering = dot(wo, n) > 0;
        Vec3 nn = entering ? n : -n;
        double eta = entering ? 1 / eta_ : eta_;
        double cosI = std::min(1.0, dot(wo, nn));
        double fr = fresnelDielectric(cosI, eta_);
        Vec3 refl = reflect(-wo, nn);
        Vec3 refr;
        if (s.uniform() < fr || !refract(-wo, nn, eta, refr)) {
            return {refl, Spectrum{1, 1, 1}, 1, true, true};
        }
        // Throughput 1 (no absorption model in this scoped build).
        return {refr, Spectrum{1, 1, 1}, 1, true, true};
    }
private:
    double eta_;
};

// GGX microfacet, specular reflection only (Cook-Torrance, F via Schlick).
class MicrofacetBSDF : public BSDF {
public:
    MicrofacetBSDF(Spectrum albedo, double roughness, double f0 = 0.04)
        : albedo_(albedo), alpha_(std::max(0.001, roughness * roughness)), f0_(f0) {}
    Spectrum f(const Vec3& wo, const Vec3& wi, const Vec3& n) const override {
        double cO = dot(wo, n), cI = dot(wi, n);
        if (cO <= 0 || cI <= 0) return {};
        Vec3 h = normalize(wo + wi);
        double d = ggxD(std::max(0.0, dot(n, h)), alpha_);
        double g = ggxG1(cO, alpha_) * ggxG1(cI, alpha_);
        double fr = fresnelSchlick(std::max(0.0, dot(wo, h)), f0_);
        return albedo_ * (d * g * fr / (4 * cO * cI));
    }
    double pdf(const Vec3& wo, const Vec3& wi, const Vec3& n) const override {
        if (dot(wo, n) <= 0 || dot(wi, n) <= 0) return 0;
        Vec3 h = normalize(wo + wi);
        double d = ggxD(std::max(0.0, dot(n, h)), alpha_);
        return d * std::max(0.0, dot(n, h)) / (4 * std::max(1e-9, dot(wo, h)));
    }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler& s) const override {
        // Sample GGX half-vector by inversion, then reflect wo about it.
        double u1 = s.uniform(), u2 = s.uniform();
        double phi = 2 * M_PI * u2;
        double cosH = std::sqrt((1 - u1) / (1 + (alpha_ * alpha_ - 1) * u1));
        double sinH = std::sqrt(std::max(0.0, 1 - cosH * cosH));
        Vec3 hLocal{sinH * std::cos(phi), sinH * std::sin(phi), cosH};
        Vec3 t = std::abs(n.z) < 0.999 ? normalize(cross(n, {0, 0, 1})) : normalize(cross(n, {1, 0, 0}));
        Vec3 b = cross(n, t);
        Vec3 h = normalize(t * hLocal.x + b * hLocal.y + n * hLocal.z);
        Vec3 wi = reflect(-wo, h);
        if (dot(wi, n) <= 0) return {};
        double p = pdf(wo, wi, n);
        if (p <= 0) return {};
        Spectrum fv = f(wo, wi, n);
        return {wi, fv * dot(wi, n) / p, p, false, true};
    }
    double alpha() const { return alpha_; }
private:
    Spectrum albedo_; double alpha_, f0_;
};

// Rough dielectric: GGX half-vector, Fresnel chooses reflection vs
// refraction (refraction direction via Snell on the half-vector side).
class RoughDielectricBSDF : public BSDF {
public:
    RoughDielectricBSDF(double eta, double roughness)
        : eta_(eta), alpha_(std::max(0.001, roughness * roughness)) {}
    Spectrum f(const Vec3&, const Vec3&, const Vec3&) const override { return {}; }  // delta-like sampling only
    double pdf(const Vec3&, const Vec3&, const Vec3&) const override { return 0; }
    BSDFSample sample(const Vec3& wo, const Vec3& n, Sampler& s) const override {
        bool entering = dot(wo, n) > 0;
        Vec3 nn = entering ? n : -n;
        double u1 = s.uniform(), u2 = s.uniform();
        double phi = 2 * M_PI * u2;
        double cosH = std::sqrt((1 - u1) / (1 + (alpha_ * alpha_ - 1) * u1));
        double sinH = std::sqrt(std::max(0.0, 1 - cosH * cosH));
        Vec3 hLocal{sinH * std::cos(phi), sinH * std::sin(phi), cosH};
        Vec3 t = std::abs(nn.z) < 0.999 ? normalize(cross(nn, {0, 0, 1})) : normalize(cross(nn, {1, 0, 0}));
        Vec3 b = cross(nn, t);
        Vec3 h = normalize(t * hLocal.x + b * hLocal.y + nn * hLocal.z);
        if (dot(h, nn) < 0) h = -h;
        double eta = entering ? 1 / eta_ : eta_;
        double fr = fresnelDielectric(std::min(1.0, dot(wo, h)), eta_);
        Vec3 refr;
        if (s.uniform() < fr || !refract(-wo, h, eta, refr)) {
            return {reflect(-wo, h), Spectrum{1, 1, 1}, 1, true, true};
        }
        return {refr, Spectrum{1, 1, 1}, 1, true, true};
    }
private:
    double eta_, alpha_;
};

}  // namespace pbr
