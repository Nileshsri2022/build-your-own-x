// Ch. 6 — Shapes (must-have: sphere, cylinder, disk, triangle mesh).
// Bilinear patches and curves are stretch goals S2 and are NOT here.
// Rounding-error behaviour (Ch. 6): triangle hits accept barycentric
// coordinates down to -1e-7, so a ray exactly on a shared edge still
// hits one of the two triangles (no crack at shared edges).
#pragma once
#include <vector>
#include <memory>
#include "geometry.h"

namespace pbr {

struct Hit {
    double t = 0; Vec3 point; Normal normal; bool frontFace = true;
};

class Shape {
public:
    virtual ~Shape() = default;
    virtual bool intersect(const Ray& r, double tMin, Hit& hit) const = 0;
    virtual Bounds bounds() const = 0;
};

class Sphere : public Shape {
public:
    Sphere(Vec3 c, double r) : c_(c), r_(r) {}
    bool intersect(const Ray& ray, double tMin, Hit& hit) const override {
        Vec3 oc = ray.origin - c_;
        double a = dot(ray.dir, ray.dir), b = dot(oc, ray.dir), cc = dot(oc, oc) - r_ * r_;
        double disc = b * b - a * cc;
        if (disc < 0) return false;
        double sq = std::sqrt(disc);
        double t = (-b - sq) / a;
        if (t < tMin || t > ray.tMax) { t = (-b + sq) / a; if (t < tMin || t > ray.tMax) return false; }
        hit.t = t; hit.point = ray.at(t);
        Vec3 n = (hit.point - c_) / r_;
        hit.frontFace = dot(ray.dir, n) < 0;
        hit.normal = Normal(hit.frontFace ? n : -n);
        return true;
    }
    Bounds bounds() const override {
        Bounds b; b.grow(c_ - Vec3{r_, r_, r_}); b.grow(c_ + Vec3{r_, r_, r_}); return b;
    }
private:
    Vec3 c_; double r_;
};

// Cylinder along the Y axis, centred at c, radius r, height h
// (y in [-h/2, h/2] relative to c), with flat end caps.
class Cylinder : public Shape {
public:
    Cylinder(Vec3 c, double r, double h) : c_(c), r_(r), h_(h) {}
    bool intersect(const Ray& ray, double tMin, Hit& hit) const override {
        Vec3 o = ray.origin - c_;
        double best = ray.tMax; Hit bh; bool found = false;
        auto consider = [&](double t, Vec3 p, Vec3 n) {
            if (t < tMin || t > best) return;
            best = t; bh.t = t; bh.point = p;
            bh.frontFace = dot(ray.dir, n) < 0; bh.normal = Normal(bh.frontFace ? n : -n); found = true;
        };
        double a = ray.dir.x * ray.dir.x + ray.dir.z * ray.dir.z;
        double b = o.x * ray.dir.x + o.z * ray.dir.z;
        double cc = o.x * o.x + o.z * o.z - r_ * r_;
        if (std::abs(a) > 1e-12) {
            double disc = b * b - a * cc;
            if (disc >= 0) {
                double sq = std::sqrt(disc);
                for (double t : {(-b - sq) / a, (-b + sq) / a}) {
                    Vec3 p = ray.at(t);
                    if (std::abs(p.y - c_.y) <= h_ / 2)
                        consider(t, p, normalize(Vec3{p.x - c_.x, 0, p.z - c_.z}));
                }
            }
        }
        for (double cap : {-h_ / 2, h_ / 2}) {
            if (std::abs(ray.dir.y) < 1e-12) continue;
            double t = (c_.y + cap - ray.origin.y) / ray.dir.y;
            Vec3 p = ray.at(t);
            double dx = p.x - c_.x, dz = p.z - c_.z;
            if (dx * dx + dz * dz <= r_ * r_)
                consider(t, p, Vec3{0, cap > 0 ? 1.0 : -1.0, 0});
        }
        if (found) hit = bh;
        return found;
    }
    Bounds bounds() const override {
        Bounds b; b.grow(c_ + Vec3{-r_, -h_ / 2, -r_}); b.grow(c_ + Vec3{r_, h_ / 2, r_}); return b;
    }
private:
    Vec3 c_; double r_, h_;
};

class Disk : public Shape {
public:
    Disk(Vec3 c, Vec3 n, double r) : c_(c), n_(normalize(n)), r_(r) {}
    bool intersect(const Ray& ray, double tMin, Hit& hit) const override {
        double denom = dot(ray.dir, n_);
        if (std::abs(denom) < 1e-12) return false;
        double t = dot(c_ - ray.origin, n_) / denom;
        if (t < tMin || t > ray.tMax) return false;
        Vec3 p = ray.at(t);
        if ((p - c_).length2() > r_ * r_) return false;
        hit.t = t; hit.point = p;
        hit.frontFace = dot(ray.dir, n_) < 0;
        hit.normal = Normal(hit.frontFace ? n_ : -n_);
        return true;
    }
    Bounds bounds() const override {
        Bounds b; b.grow(c_ - Vec3{r_, r_, r_}); b.grow(c_ + Vec3{r_, r_, r_}); return b;
    }
private:
    Vec3 c_, n_; double r_;
};

struct Triangle { Vec3 v0, v1, v2; };

class TriangleMesh : public Shape {
public:
    explicit TriangleMesh(std::vector<Triangle> tris) : tris_(std::move(tris)) {}
    bool intersect(const Ray& ray, double tMin, Hit& hit) const override {
        // Möller–Trumbore; barycentric tolerance -1e-7 closes shared edges.
        const double eps = 1e-7;
        bool found = false; double best = ray.tMax;
        for (const auto& tri : tris_) {
            Vec3 e1 = tri.v1 - tri.v0, e2 = tri.v2 - tri.v0;
            Vec3 pv = cross(ray.dir, e2);
            double det = dot(e1, pv);
            if (std::abs(det) < 1e-12) continue;
            double inv = 1 / det;
            Vec3 tv = ray.origin - tri.v0;
            double u = dot(tv, pv) * inv;
            if (u < -eps || u > 1 + eps) continue;
            Vec3 qv = cross(tv, e1);
            double v = dot(ray.dir, qv) * inv;
            if (v < -eps || u + v > 1 + eps) continue;
            double t = dot(e2, qv) * inv;
            if (t < tMin || t > best) continue;
            Vec3 n = normalize(cross(e1, e2));
            best = t; hit.t = t; hit.point = ray.at(t);
            hit.frontFace = dot(ray.dir, n) < 0;
            hit.normal = Normal(hit.frontFace ? n : -n); found = true;
        }
        return found;
    }
    Bounds bounds() const override {
        Bounds b;
        for (const auto& tri : tris_) { b.grow(tri.v0); b.grow(tri.v1); b.grow(tri.v2); }
        return b;
    }
    size_t count() const { return tris_.size(); }
private:
    std::vector<Triangle> tris_;
};

}  // namespace pbr
