#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
  public:
    sphere(const point3& center, double radius, shared_ptr<material> mat)
        : center(center), radius(std::fmax(0, radius)), mat(mat) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Ray-sphere quadratic, written with oc = origin - center:
        //   a t^2 + 2 b t + c = 0, with a = dir.dir, b = oc.dir,
        //   c = oc.oc - radius^2. Discriminant b^2 - a c decides
        // hit vs miss; the nearer root inside ray_t is the hit.
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = b * b - a * c;

        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        auto root = (-b - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (-b + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 center;
    double radius;
    shared_ptr<material> mat;
};

#endif
