#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "hittable_list.h"
#include "interval.h"
#include "vec3.h"

#include "model.h"

class triangle : public hittable {
    public:
        triangle(point3 v0, point3 v1, point3 v2, std::shared_ptr<material> material)
         : v{v0, v1, v2}, mat{material} 
        {
            vec3 v0v1 = v1 - v0;
            vec3 v0v2 = v2 - v0;
            normal = cross(v0v1, v0v2);
            D = -dot(normal, v0);
        }

        virtual bool hit(
            const ray& r, interval ray_t, hit_record& rec) const override;

    private:
        point3 v[3];
        vec3 normal;
        double D;
        std::shared_ptr<material> mat;
};

/* Two Steps:
   1. Solve ray plane intersection equation:
   Check dot(dir, normal) != 0 (if 0 ray and plane are parallel, return false)
   Otherwise: t = - (dot(origin, normal) + D/dot(dir, normal))
   If t is negative triangle is behind ray, return false
   2. Solve inside-outside test for intersection point:
   For edge e_n = v_n to v(n+1)%3:
   - Calculate edge v_n to p e_p = p - v_n
   - Compute cross product N_p = e_n x e_p
   - Compute dot product of N and N_p
   - Point is in triangle if dot product is positive for all edges
*/
bool triangle::hit(const ray& r, interval ray_t, hit_record& rec) const {
    double RdotN = dot(r.dir, normal);
    if (RdotN == 0) return false;
    double t = - (dot(r.orig, normal) + D)/RdotN;
    if (t < 0) return false;

    point3 p = r.at(t);

    vec3 v0v1 = v[1] - v[0];
    vec3 v1v2 = v[2] - v[1];
    vec3 v2v0 = v[0] - v[2];
    vec3 v0p = p - v[0];
    vec3 v1p = p - v[1];
    vec3 v2p = p - v[2];

    if (!(dot(cross(v0v1, v0p), normal) > 0 &&
          dot(cross(v1v2, v1p), normal) > 0 &&
          dot(cross(v2v0, v2p), normal) > 0)) return false;
    
    rec.t = t;
    rec.p = p;
    rec.set_face_normal(r, normal);
    rec.mat = mat;
    
    return true;
}

void mesh_to_hittables(Model model, hittable_list hittables) {

}

#endif // TRIANGLE_H
