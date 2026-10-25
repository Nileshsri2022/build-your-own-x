// Ch. 3 — Geometry and Transformations (our own structure, not book code).
// All math in double precision. Vec3 is used for vectors, points and
// colours (Spectrum is an alias in radiometry.h). Normal is a separate
// type because normals transform differently from points/vectors.
#pragma once
#include <cmath>
#include <algorithm>
#include <limits>

namespace pbr {

struct Vec3 {
    double x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    double length2() const { return x * x + y * y + z * z; }
    double length() const { return std::sqrt(length2()); }
};
inline Vec3 operator*(double s, const Vec3& v) { return v * s; }
inline Vec3 operator*(const Vec3& a, const Vec3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
// Documented decision (TEST_PLAN: zero vector must not become NaN):
// normalising the zero vector returns the zero vector.
inline Vec3 normalize(const Vec3& v) {
    double l = v.length();
    return l == 0 ? Vec3{} : v / l;
}
inline Vec3 reflect(const Vec3& d, const Vec3& n) { return d - 2 * dot(d, n) * n; }
// Returns false on total internal reflection.
inline bool refract(const Vec3& d, const Vec3& n, double eta, Vec3& out) {
    double cosI = std::min(1.0, dot(-d, n));
    double sin2 = eta * eta * (1 - cosI * cosI);
    if (sin2 > 1) return false;
    out = eta * d + (eta * cosI - std::sqrt(1 - sin2)) * n;
    return true;
}

struct Normal {
    double x = 0, y = 0, z = 0;
    Normal() = default;
    Normal(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    explicit Normal(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}
    Vec3 asVec() const { return {x, y, z}; }
};

struct Ray {
    Vec3 origin, dir;  // dir is assumed normalised by callers
    double tMax = std::numeric_limits<double>::infinity();
    Vec3 at(double t) const { return origin + dir * t; }
};

struct Bounds {
    Vec3 pMin{std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity()};
    Vec3 pMax{-std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity()};
    void grow(const Vec3& p) {
        pMin.x = std::min(pMin.x, p.x); pMin.y = std::min(pMin.y, p.y); pMin.z = std::min(pMin.z, p.z);
        pMax.x = std::max(pMax.x, p.x); pMax.y = std::max(pMax.y, p.y); pMax.z = std::max(pMax.z, p.z);
    }
    void grow(const Bounds& b) { grow(b.pMin); grow(b.pMax); }
    Vec3 centroid() const { return (pMin + pMax) * 0.5; }
    int longestAxis() const {
        Vec3 d = pMax - pMin;
        return d.x > d.y ? (d.x > d.z ? 0 : 2) : (d.y > d.z ? 1 : 2);
    }
    // Slab test; also true for a ray starting inside the box.
    bool hit(const Ray& r, double tMin, double tMax) const {
        double lo = tMin, hi = tMax;
        const double o[3] = {r.origin.x, r.origin.y, r.origin.z};
        const double d[3] = {r.dir.x, r.dir.y, r.dir.z};
        const double mn[3] = {pMin.x, pMin.y, pMin.z};
        const double mx[3] = {pMax.x, pMax.y, pMax.z};
        for (int i = 0; i < 3; ++i) {
            if (std::abs(d[i]) < 1e-12) {
                if (o[i] < mn[i] || o[i] > mx[i]) return false;
            } else {
                double inv = 1 / d[i], t0 = (mn[i] - o[i]) * inv, t1 = (mx[i] - o[i]) * inv;
                if (t0 > t1) std::swap(t0, t1);
                lo = std::max(lo, t0); hi = std::min(hi, t1);
                if (lo > hi) return false;
            }
        }
        return true;
    }
};

// Affine transform (translation + rotation + scale) as a 4x4 matrix.
// Points transform with the matrix; vectors without translation;
// normals with the inverse-transpose (implemented via the stored
// inverse matrix for the rotation/scale part).
struct Transform {
    double m[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    double inv[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    static Transform translate(const Vec3& t) {
        Transform tr; tr.m[0][3] = t.x; tr.m[1][3] = t.y; tr.m[2][3] = t.z;
        tr.inv[0][3] = -t.x; tr.inv[1][3] = -t.y; tr.inv[2][3] = -t.z; return tr;
    }
    static Transform scale(double sx, double sy, double sz) {
        Transform tr; tr.m[0][0] = sx; tr.m[1][1] = sy; tr.m[2][2] = sz;
        tr.inv[0][0] = 1 / sx; tr.inv[1][1] = 1 / sy; tr.inv[2][2] = 1 / sz; return tr;
    }
    static Transform rotateY(double a) {
        Transform tr; double c = std::cos(a), s = std::sin(a);
        tr.m[0][0] = c; tr.m[0][2] = s; tr.m[2][0] = -s; tr.m[2][2] = c;
        tr.inv[0][0] = c; tr.inv[0][2] = -s; tr.inv[2][0] = s; tr.inv[2][2] = c; return tr;
    }
    Transform operator*(const Transform& o) const {
        Transform r;
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) {
            r.m[i][j] = 0; r.inv[i][j] = 0;
            for (int k = 0; k < 4; ++k) { r.m[i][j] += m[i][k] * o.m[k][j]; r.inv[i][j] += o.inv[i][k] * inv[k][j]; }
        }
        return r;
    }
    Vec3 point(const Vec3& p) const {
        return {m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3],
                m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3],
                m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]};
    }
    Vec3 vector(const Vec3& v) const {
        return {m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
                m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
                m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z};
    }
    Normal normal(const Normal& n) const {
        // inverse-transpose of the linear part: inv^T * n
        Vec3 v{inv[0][0]*n.x + inv[1][0]*n.y + inv[2][0]*n.z,
               inv[0][1]*n.x + inv[1][1]*n.y + inv[2][1]*n.z,
               inv[0][2]*n.x + inv[1][2]*n.y + inv[2][2]*n.z};
        return Normal(normalize(v));
    }
};

}  // namespace pbr
