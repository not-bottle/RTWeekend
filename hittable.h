#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "vec2.h"

#include "aabb.h"

class material;

struct hit_record {
    point3 p; // Point at which the ray has hit an object
    vec3 normal; // Unit normal vector at the point
    std::shared_ptr<material> mat; // The material of the object hit
    double t; // Parameter at which the ray intersects
    bool front_face; // True if ray is intersecting from "outside" of object. 
    //                  False if ray is intersecting from inside.
    double u, v; // UV coords (mapping determined by object type)

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

        virtual double pdf_value(const point3& origin, const vec3& direction) const
        {
            return 0.0;
        }

        virtual vec3 random(const point3& origin) const
        {
            return vec3(1,0,0);
        }

        virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
    public:
        translate(std::shared_ptr<hittable> object, const vec3& offset)
          : object(object), offset(offset)
        {
            bbox = object->bounding_box() + offset;
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // Move ray backwards by the offset
            ray offset_r(r.origin() - offset, r.direction(), r.time());

            // Determine whether an intersection exists along the offset ray
            if (!object->hit(offset_r, ray_t, rec))
                return false;
            
            // Move intersection point forwards by the offset
            rec.p += offset;

            return true;
        }

        aabb bounding_box() const override { return bbox; }
    
    private:
        std::shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
};

class rotate_y : public hittable {
    public:
        rotate_y(std::shared_ptr<hittable> object, double angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            // Iterate over all corners of the bounding box, rotate them and test for new
            // maximums and minimums for each of x,y,z
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-i)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-i)*bbox.z.min;

                        auto newx =  cos_theta*x + sin_theta*z;
                        auto newz = -sin_theta*x + cos_theta*z;
                        
                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray&r, interval ray_t, hit_record& rec) const override {

            // Transform ray from world space to object space.

            auto origin = point3(
                (cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
                r.origin().y(),
                (sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
            );

            auto direction = vec3(
                (cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
                r.direction().y(),
                (sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
            );

            ray rotated_r(origin, direction, r.time());

            // Determine whether an intersection exists in object space (and if so, where)

            if (!object->hit(rotated_r, ray_t, rec))
                return false;

            // Transform the intersection from object space back to world space.

            rec.p = point3(
                (cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
                rec.p.y(),
                (-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
            );

            rec.normal = vec3(
                (cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
                rec.normal.y(),
                (-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        std::shared_ptr<hittable> object;
        double cos_theta;
        double sin_theta;
        aabb bbox;
};

#endif
