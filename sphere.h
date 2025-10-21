#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"
#include "interval.h"

class sphere : public hittable {
    public:
        sphere() {}
        sphere(const point3& centre, double radius, std::shared_ptr<material> material)
         : centre(centre, vec3()), radius(radius), mat(material) 
        {
            auto rvec = vec3(radius, radius, radius);
            bbox = aabb(centre - rvec, centre + rvec);
        }

        sphere(const point3& centre0, const point3& centre1, double radius, std::shared_ptr<material> material)
         : centre(centre0, centre1 - centre0), radius(radius), mat(material) 
        {
            auto rvec = vec3(radius, radius, radius);
            aabb box1(centre.at(0) - rvec, centre.at(0) + rvec);
            aabb box2(centre.at(1) - rvec, centre.at(1) + rvec);
            bbox = aabb(box1, box2);
        }

        virtual bool hit(
            const ray& r, interval ray_t, hit_record& rec) const override;

        aabb bounding_box() const override { return bbox; }

    private:
        ray centre;
        double radius;
        std::shared_ptr<material> mat;
        aabb bbox;
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
    point3 current_centre = centre.at(r.time());
    vec3 oc = r.origin() - current_centre;
    auto a = r.direction().length_squared();
    auto half_b = dot(r.direction(), oc);
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
    vec3 outward_normal = (rec.p - current_centre) / radius; // Calculate unit normal (made unit by dividing by radius)
    rec.set_face_normal(r, outward_normal); // Determines if the ray is on the inside or outside of the sphere.
    //                                         Flips normal to face ray if on inside. 
    rec.mat = mat;

    return true;
};

#endif

