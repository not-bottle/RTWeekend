#ifndef RAY_H
#define RAY_H

#include "vec3.h"

/* Generic ray class. Used to construct a "ray" in V(t) = orig + t*dir format. */

class ray {
    public:
        ray() {}
        ray(const point3& origin, const vec3& direction, double time)
            : orig(origin), dir(direction), tm(time) {}

        ray(const point3& origin, const vec3& direction)
            : ray(origin, direction, 0) {}

        point3 origin() const  { return orig; }
        vec3 direction() const { return dir; }

        double time() const { return tm; }

        // Returns position along ray
        point3 at(double t) const {
            return orig + t*dir;
        }

    public:
        point3 orig;
        vec3 dir;
        double tm;
};

#endif