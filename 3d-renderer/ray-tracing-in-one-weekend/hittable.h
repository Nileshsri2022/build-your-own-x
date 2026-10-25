#ifndef HITTABLE_H
#define HITTABLE_H

#include "interval.h"
#include "ray.h"

class material;

// Everything a ray needs to know about one hit: where, how far
// along the ray, which way the surface faces, and what it is made of.
class hit_record {
  public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    bool front_face;

    // The stored normal always faces against the incoming ray, so a
    // ray entering a hollow glass sphere sees the inner surface as a
    // back face and refraction flips its index ratio accordingly.
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
  public:
    virtual ~hittable() = default;

    // True + filled record when the ray hits within ray_t; false
    // otherwise. Implementations only report the nearest hit inside
    // the interval they are given.
    virtual bool hit(const ray& r, interval ray_t,
                     hit_record& rec) const = 0;
};

#endif
