#ifndef RAY_H
#define RAY_H

#include "vec3.h"

/* Generic ray class. Used to construct a "ray" in V(t) = orig + t*dir format. */

class ray {
    public:
        ray() {}
        ray(const point3& origin, const vec3& direction)
            : orig(origin), dir(direction)
        {}

        point3 origin() const  { return orig; }
        vec3 direction() const { return dir; }

        // Returns position along ray
        point3 at(double t) const {
            return orig + t*dir;
        }

    public:
        point3 orig;
        vec3 dir;
};

#endif