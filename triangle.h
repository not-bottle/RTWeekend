#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "hittable_list.h"
#include "interval.h"
#include "vec3.h"

#include "model.h"

class triangle : public hittable {
    public:
        triangle(point3 v0, point3 v1, point3 v2, std::shared_ptr<material> material, vec3 direction)
         : v{v0, v1, v2}, mat{material}, direction{direction}
        {
            vec3 v0v1 = v1 - v0;
            vec3 v0v2 = v2 - v0;
            normal = cross(v0v1, v0v2);
            D = -dot(normal, v0);
        }

        triangle(point3 v0, point3 v1, point3 v2, std::shared_ptr<material> material)
         : triangle(v0, v1, v2, material, vec3()) {}

        virtual bool hit(
            const ray& r, interval ray_t, hit_record& rec) const override;

    private:
        point3 v[3];
        vec3 normal;
        double D;
        std::shared_ptr<material> mat;
        vec3 direction{0, 0, 0};

        bool hit_geometric(const ray& r, interval ray_bounds, hit_record& rec) const;
        bool hit_geometric_smooth(const ray& r, interval ray_bounds, hit_record& rec) const;
        bool hit_moller_trumbore(const ray& r, interval ray_bounds, hit_record& rec) const;
};


bool triangle::hit(const ray& r, interval ray_bounds, hit_record& rec) const {
    return hit_moller_trumbore(r, ray_bounds, rec);
}

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
bool triangle::hit_geometric(const ray& r, interval ray_bounds, hit_record& rec) const {
    double RdotN = dot(r.dir, normal);
    if (RdotN == 0) return false;
    double t = - (dot(r.orig, normal) + D)/RdotN;
    if (t < 0) return false;
    if (!ray_bounds.surrounds(t)) return false;

    point3 p = r.at(t);

    vec3 v0v1 = v[1] - v[0];
    vec3 v1v2 = v[2] - v[1];
    vec3 v2v0 = v[0] - v[2];
    vec3 v0p = p - v[0];
    vec3 v1p = p - v[1];
    vec3 v2p = p - v[2];

    vec3 c0 = cross(v0v1, v0p);
    vec3 c1 = cross(v1v2, v1p);
    vec3 c2 = cross(v2v0, v2p);

    double ndotc0 = dot(c0, normal);
    double ndotc1 = dot(c1, normal);
    double ndotc2 = dot(c2, normal);

    if (!(ndotc0 > 0 &&
          ndotc1 > 0 &&
          ndotc2 > 0)) return false;

    double denom = dot(normal, normal);
    double u = ndotc1 / denom;
    double v = ndotc2 / denom;

    rec.t = t;
    rec.p = p;
    rec.set_face_normal(r, unit_vector(normal));
    rec.mat = mat;
    rec.uv = vec2(u, v);
    
    return true;
}

/* For a triangle with vertices A, B, C, and ray P = O + tD:
   Barycentric Equation Rearranged in terms of t, u, v: 
     (O-A) = -Dt + u(B-A) + v(C-A)
   Let:
     T = (O-A)
     E1 = (B-A)
     E2 = (C-A)
   Using Cramer's Rule:
     [t, u, v] = 1/det([-D E1 E2])[det(T E1 E2) det(-D T E2) det(-D E1 T)]
   We can calculate determinants using scalar triple products (rearranged to minimize computation):
     det(M)  = det([-D E1 E2]) = dot(D x E2, E1)
     det(Mt) = det([T E1 E2])  = dot(T x E1, E2)
     det(Mu) = det([-D T E2])  = dot(D x E2, T)
     det(Mv) = det([-D E1 T])  = dot(T x E1, D)
   Shared cross products: D x E2, T x E1
   Some Rules:
   - If det(M) is 0 ray and triangle are parallel
   - If det(M) < 0 ray is backfacing
*/
bool triangle::hit_moller_trumbore(const ray& r, interval ray_bounds, hit_record& rec) const {
    point3 v0 = v[0] + r.time()*direction;
    point3 v1 = v[1] + r.time()*direction;
    point3 v2 = v[2] + r.time()*direction;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 dxe2 = cross(r.dir, e2);
    double detM = dot(dxe2, e1);

    if (detM == 0) return false;
    
    double invDet = 1/detM;
    vec3 T = (r.orig - v0);
    double u = dot(dxe2, T) * invDet;
    if (u < 0 || u > 1) return false;

    vec3 txe1 = cross(T, e1);
    double v = dot(txe1, r.dir) * invDet;
    if (v < 0 || u + v > 1) return false;

    double t = dot(txe1, e2) * invDet;

    if (t < 0 || !ray_bounds.surrounds(t)) return false;

    rec.t = t;
    rec.p = r.at(t);
    rec.set_face_normal(r, unit_vector(normal));
    rec.mat = mat;
    rec.uv = vec2(u, v);
    
    return true;
}


void mesh_to_hittables(Model &model, hittable_list &hittables, std::shared_ptr<material> mat, vec3 direction) {
    std::cerr << "Num meshes:" << model.meshes.size() << std::endl;
    for (int m = 0; m < model.meshes.size(); m++) {
        Mesh mesh = model.meshes[m];
        int acc = 0;
        for (int i = 0; i < mesh.indices.size(); i += 3) {
            point3 v0 = mesh.vertices[mesh.indices[i]].Position;
            point3 v1 = mesh.vertices[mesh.indices[i+1]].Position;
            point3 v2 = mesh.vertices[mesh.indices[i+2]].Position;
            std::shared_ptr<triangle> tri = std::make_shared<triangle>(triangle{v0, v1, v2, mat, direction});
            hittables.add(tri);
            acc += 1;
        }
        std::cerr << "Num triangles in mesh: " << acc << std::endl;
    }
}

#endif // TRIANGLE_H
