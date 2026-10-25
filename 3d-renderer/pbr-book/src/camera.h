// Ch. 5 — Cameras (projective camera only; spherical camera is a
// stretch goal S1 and is NOT built here).
#pragma once
#include "geometry.h"

namespace pbr {

class ProjectiveCamera {
public:
    ProjectiveCamera(Vec3 eye, Vec3 lookAt, Vec3 up, double fovDeg, double aspect)
        : eye_(eye), aspect_(aspect) {
        forward_ = normalize(lookAt - eye);
        right_ = normalize(cross(forward_, up));
        up_ = cross(right_, forward_);
        double tanHalf = std::tan(fovDeg * M_PI / 360.0);
        tanHalf_ = tanHalf;
        fovDeg_ = fovDeg;
    }
    // u, v in [0,1] across the image (u right, v up). Ray through the
    // centre (0.5, 0.5) points exactly along forward_.
    Ray generateRay(double u, double v) const {
        double x = (2 * u - 1) * aspect_ * tanHalf_;
        double y = (2 * v - 1) * tanHalf_;
        return {eye_, normalize(forward_ + right_ * x + up_ * y)};
    }
    double fovDeg() const { return fovDeg_; }
    Vec3 forward() const { return forward_; }
private:
    Vec3 eye_, forward_, right_, up_;
    double aspect_, tanHalf_, fovDeg_;
};

}  // namespace pbr
