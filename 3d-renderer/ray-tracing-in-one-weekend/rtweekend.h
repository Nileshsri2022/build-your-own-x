#ifndef RTWEEKEND_H
#define RTWEEKEND_H

// Shared constants and random-number helpers for the whole renderer.
// Standard library only. The generator is default-seeded, as in the
// book's listings, so repeated runs of one binary reproduce the same
// image; tests therefore use tolerances, not byte equality.

#include <cmath>
#include <limits>
#include <memory>
#include <random>

using std::make_shared;
using std::shared_ptr;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

#endif
