#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"

// Linear -> gamma space for gamma 2: the inverse transform is sqrt.
// Negative components (which can only arise from numeric noise) map
// to 0 instead of producing NaN.
inline double linear_to_gamma(double linear_component) {
    if (linear_component > 0)
        return std::sqrt(linear_component);
    return 0;
}

// Write one averaged pixel colour as an ASCII PPM triple. The
// caller has already divided the summed samples by the sample count.
// Bytes come from clamping to [0, 0.999] and scaling by 256.
inline void write_color(std::ostream& out, const color& pixel_color) {
    auto r = linear_to_gamma(pixel_color.x());
    auto g = linear_to_gamma(pixel_color.y());
    auto b = linear_to_gamma(pixel_color.z());

    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif
