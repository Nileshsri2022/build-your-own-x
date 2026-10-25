// geometry.h — vectors, matrices, barycentric helpers (own implementation).
// All geometry math in double. Screen space: x right, y DOWN (row 0 = top),
// which is why a world-space CCW triangle viewed from +z has negative
// signed area in screen space (see renderer.cpp culling rule).
#pragma once
#include <cmath>

struct Vec2 {
    double x = 0, y = 0;
    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s) const { return {x * s, y * s}; }
};

struct Vec3 {
    double x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }
};

inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline double length(const Vec3& a) { return std::sqrt(dot(a, a)); }
inline Vec3 normalized(const Vec3& a) {
    double l = length(a);
    return l == 0 ? a : a / l;
}

// Row-major 4x4 matrix, column vectors: clip = M * (x, y, z, 1).
struct Mat4 {
    double m[4][4] = {};
    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; i++) r.m[i][i] = 1;
        return r;
    }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            double s = 0;
            for (int k = 0; k < 4; k++) s += a.m[i][k] * b.m[k][j];
            r.m[i][j] = s;
        }
    return r;
}

// Transform a point (w = 1); returns xyzw.
inline void transformPoint(const Mat4& M, const Vec3& p, double out[4]) {
    double v[4] = {p.x, p.y, p.z, 1.0};
    for (int i = 0; i < 4; i++)
        out[i] = M.m[i][0] * v[0] + M.m[i][1] * v[1] + M.m[i][2] * v[2] + M.m[i][3] * v[3];
}

inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = normalized(center - eye);
    Vec3 r = normalized(cross(f, up));
    Vec3 u = cross(r, f);
    Mat4 v = Mat4::identity();
    v.m[0][0] = r.x; v.m[0][1] = r.y; v.m[0][2] = r.z; v.m[0][3] = -dot(r, eye);
    v.m[1][0] = u.x; v.m[1][1] = u.y; v.m[1][2] = u.z; v.m[1][3] = -dot(u, eye);
    v.m[2][0] = -f.x; v.m[2][1] = -f.y; v.m[2][2] = -f.z; v.m[2][3] = dot(f, eye);
    return v;
}

inline Mat4 perspective(double fovDeg, double aspect, double near_, double far_) {
    double f = 1.0 / std::tan(fovDeg * 3.141592653589793 / 360.0);
    Mat4 p;
    p.m[0][0] = f / aspect;
    p.m[1][1] = f;
    p.m[2][2] = (far_ + near_) / (near_ - far_);
    p.m[2][3] = 2 * far_ * near_ / (near_ - far_);
    p.m[3][2] = -1;
    return p;
}

// Barycentric weights of p w.r.t. screen triangle (a, b, c), via edge
// functions. Returns false when the triangle is degenerate (area ~ 0).
// Weights are >= 0 exactly for points inside (edge included).
inline bool barycentric(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& p, double w[3]) {
    double area2 = a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
    if (std::fabs(area2) < 1e-12) return false;
    w[0] = (p.x * (b.y - c.y) + b.x * (c.y - p.y) + c.x * (p.y - b.y)) / area2;
    w[1] = (p.x * (c.y - a.y) + c.x * (a.y - p.y) + a.x * (p.y - c.y)) / area2;
    w[2] = 1.0 - w[0] - w[1];
    return true;
}

// Signed doubled area of a screen-space triangle (same formula as above).
inline double signedArea2(const Vec2& a, const Vec2& b, const Vec2& c) {
    return a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
}
