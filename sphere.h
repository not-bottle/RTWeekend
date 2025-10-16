#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"
#include "interval.h"

class sphere : public hittable {
    public:
        sphere() {}
        sphere(point3 _centre, double _radius, std::shared_ptr<material> _material)
         : centre(_centre), radius(_radius), mat(_material) {}

        virtual bool hit(
            const ray& r, interval ray_t, hit_record& rec) const override;

    private:
        point3 centre;
        double radius;
        std::shared_ptr<material> mat;
};

/* Solves the following quadratic equation: 
   (-b +- sqrt(b^2 - 4ac))/2a
   Where:
   b = -2*ray.direction . (ray.origin - sphere.centre)
   a = ray.direction . ray.direction
   c = (ray.direction - sphere.centre) . (ray.direction - sphere.centre) - sphere.radius^2

   The results are interpreted as follows:
   - No solution - the ray does not intercept the sphere
   - One solution - the ray glances off the side of the sphere
   - Two solutions - the ray passes through the sphere (hitting a point on either side)
   */
bool sphere::hit(const ray& r, interval ray_bounds, hit_record& rec) const {
    vec3 oc = r.origin() - centre;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (!ray_bounds.surrounds(root)) {
        root = (-half_b + sqrtd) / a;
            if (!ray_bounds.surrounds(root))
            return false;
    }

    rec.t = root; // Store value t in the hit record
    rec.p = r.at(rec.t); // Store the hit point
    vec3 outward_normal = (rec.p - centre) / radius; // Calculate unit normal (made unit by dividing by radius)
    rec.set_face_normal(r, outward_normal); // Determines if the ray is on the inside or outside of the sphere.
    //                                         Flips normal to face ray if on inside. 
    rec.mat = mat;

    return true;
};

#endif

