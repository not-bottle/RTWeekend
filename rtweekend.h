#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>

#include "vec3.h"
#include "random_help.h"

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}


vec3 random_in_unit_sphere() 
{
    while (true) {
        // Reject vectors that fall outside unit sphere
        auto p = vec3::random(-1, 1);
        if (p.length_squared() < 1) {
            return p;
        }
    }
}

vec3 random_in_unit_disk() 
{
    while (true) {
        // Reject vectors outside the unit disk
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() < 1) {
            return p;
        }
    }
}

vec3 random_unit_vector() 
{
    return unit_vector(random_in_unit_sphere());
}

vec3 random_on_hemisphere(const vec3 &normal) 
{
    vec3 on_unit_sphere = random_unit_vector();
    // Invert vector if not facing same direction as surface normal (pointing into sphere)
    if (dot(on_unit_sphere, normal) > 0.0) {
        return on_unit_sphere;
    } else {
        return -on_unit_sphere;
    }
}

/* 
 * Sample a random direction from sphere's hemisphere
 * relative to z axis using pdf = cos(theta)/pi.
 * Finding the CDF for the hemisphere using the pdf
 * gives the coordinate component values.
 */
inline vec3 random_cosine_direction() {
    auto r1 = random_double();
    auto r2 = random_double();

    auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);
    auto z = std::sqrt(1-r2);

    return vec3(x, y, z);
}

// Type Aliases
using point3 = vec3;
using colour = vec3;

// Common Headers

#include "ray.h"
#include "interval.h"

#endif
