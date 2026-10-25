#ifndef INTERVAL_H
#define INTERVAL_H

#include "rtweekend.h"

class interval {
  public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {}  // Empty interval.
    interval(double min, double max) : min(min), max(max) {}

    double size() const { return max - min; }

    bool contains(double x) const { return min <= x && x <= max; }

    // Strictly inside: endpoints do not count. Hit acceptance uses
    // this so a ray never re-hits the surface it just left at t = 0.
    bool surrounds(double x) const { return min < x && x < max; }

    double clamp(double x) const {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }

    static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif
