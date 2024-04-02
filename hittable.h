#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

struct hit_record {
    point3 p; // Point at which the ray has hit an object
    vec3 normal; // Unit normal vector at the point
    double t; // Parameter at which the ray intersects
    bool front_face; // True if ray is intersecting from "outside" of object. 
    //                  False if ray is intersecting from inside.

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Determine if ray is facing the inside or outside of the surface by
        // seeing if the dot product is negative. 
        // (If it is negative the outside normal is facing opposite the ray, which means the ray is on the outside)
        front_face = dot(r.direction(), outward_normal) < 0; 
        // If the ray is inside the surface, flip the normal so it is facing the ray (normals will always point towards the ray)
        normal = front_face ? outward_normal : -outward_normal; 
    }
};

class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

#endif