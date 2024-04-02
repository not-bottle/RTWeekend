#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"
#include "interval.h"

class sphere : public hittable {
    public:
        sphere() {}
        sphere(point3 cen, double r) : center(cen), radius(r) {};

        virtual bool hit(
            const ray& r, interval ray_t, hit_record& rec) const override;

    public:
        point3 center;
        double radius;
};

/* Solves the following quadratic equation: 
   (-b +- sqrt(b^2 - 4ac))/2a
   Where:
   b = 2*ray.direction . (ray.origin - sphere.centre)
   a = ray.direction . ray.direction
   c = (ray.direction - sphere.centre) . (ray.direction - sphere.centre) - sphere.radius^2

   The results are interpreted as follows:
   - No solution - the ray does not intercept the sphere
   - One solution - the ray glances off the side of the sphere
   - Two solutions - the ray passes through the sphere (hitting a point on either side)
   */
bool sphere::hit(const ray& r, interval ray_t, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (!ray_t.surrounds(root)) {
        root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root))
            return false;
    }

    rec.t = root; // Store value t in the hit record
    rec.p = r.at(rec.t); // Store the hit point
    vec3 outward_normal = (rec.p - center) / radius; // Calculate unit normal (made unit by dividing by radius)
    rec.set_face_normal(r, outward_normal); // Determines if the ray is on the inside or outside of the sphere.
    //                                         Flips normal to face ray if on inside. 

    return true;
};

#endif

