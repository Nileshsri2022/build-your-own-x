#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material {
  public:
    virtual ~material() = default;

    // Produce a scattered ray, or return false when the ray is
    // absorbed. On success, attenuation is the fraction of colour
    // the surface keeps per bounce.
    virtual bool scatter(const ray&, const hit_record&, color&,
                         ray&) const {
        return false;
    }
};

// Matte surface: scatter toward normal + random unit vector, keep
// the albedo fraction of the light. A near-zero scatter direction
// falls back to the normal itself.
class lambertian : public material {
  public:
    lambertian(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray&, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

  private:
    color albedo;
};

// Mirror surface: reflect about the normal. Fuzz (clamped to [0,1])
// perturbs the reflection with a random unit vector, blurring the
// mirror. A reflection that goes below the surface is absorbed.
class metal : public material {
  public:
    metal(const color& albedo, double fuzz)
        : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};

// Glass surface: no colour loss (white attenuation). Entering from
// air uses ratio 1/refraction_index, exiting uses refraction_index.
// The ray reflects when Snell's law has no solution (total internal
// reflection) or when Schlick's reflectance probability, sampled
// randomly, says reflect; otherwise it refracts.
class dielectric : public material {
  public:
    dielectric(double refraction_index)
        : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0 / refraction_index)
                                   : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction);
        return true;
    }

  private:
    double refraction_index;

    // Schlick's polynomial approximation of Fresnel reflectance.
    static double reflectance(double cosine, double refraction_index) {
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

#endif
