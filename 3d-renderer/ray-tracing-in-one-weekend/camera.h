#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>

class camera {
  public:
    double aspect_ratio = 1.0;       // Width over height.
    int image_width = 100;           // Rendered width in pixels.
    int samples_per_pixel = 10;      // Random samples per pixel.
    int max_depth = 10;              // Maximum ray bounces.
    double vfov = 90;                // Vertical field of view, degrees.
    point3 lookfrom = point3(0, 0, 0);   // Camera position.
    point3 lookat = point3(0, 0, -1);    // Point camera looks at.
    vec3 vup = vec3(0, 1, 0);            // Camera-relative up.
    double defocus_angle = 0;        // Cone angle of lens, degrees.
    double focus_dist = 10;          // Distance to plane in focus.

    // PPM (P3) to stdout, progress to stderr so the image stays
    // clean even when stdout is piped to a file.
    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height
                  << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j)
                      << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

  private:
    int image_height;            // Rendered height in pixels.
    double pixel_samples_scale;  // 1 / samples_per_pixel.
    point3 center;               // Camera center (= lookfrom).
    point3 pixel00_loc;          // Location of pixel 0, 0.
    vec3 pixel_delta_u;          // Offset to pixel to the right.
    vec3 pixel_delta_v;          // Offset to pixel below.
    vec3 u, v, w;                // Camera frame basis vectors.
    vec3 defocus_disk_u;         // Defocus disk horizontal radius.
    vec3 defocus_disk_v;         // Defocus disk vertical radius.

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Viewport dimensions from vertical fov and focus distance.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width =
            viewport_height * (double(image_width) / image_height);

        // Camera frame: w points opposite the view direction, u to
        // the right, v up.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        auto viewport_upper_left =
            center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Defocus disk: radius from the defocus cone angle at the
        // focus distance. Zero angle means a pinhole (no blur).
        auto defocus_radius =
            focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // One jittered ray through pixel (i, j): a random point in the
    // pixel square, fired from a random point on the defocus disk
    // (or the camera center when defocus is off).
    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) +
                            ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }

    // Random point in the [-0.5, +0.5] square around the pixel.
    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;

        // 0.001 lower bound: a scattered ray must not immediately
        // re-hit its own surface. This is a numerical guard, not a
        // shadow feature — this renderer has no lights or shadows.
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        // Sky gradient: white at the bottom, blue (0.5, 0.7, 1.0) at
        // the top, blended by the ray's normalised y-component.
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif
