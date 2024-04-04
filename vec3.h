#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

/* Generic class to hold a 3D vector. Defines common operations. */

class vec3 {
    public:
        double e[3]; // 3D Vector

        vec3() : e{0,0,0} {}
        vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

        double x() const { return e[0]; }
        double y() const { return e[1]; }
        double z() const { return e[2]; }

        vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
        double operator[](int i) const { return e[i]; }
        double& operator[](int i) { return e[i]; }

        vec3& operator+=(const vec3 &v) {
            e[0] += v[0];
            e[1] += v[1];
            e[2] += v[2];
            return *this;
        }

        vec3& operator*=(double t) {
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;
            return *this;
        }

        vec3& operator/=(double t) {
            return *this *= 1/t;
        }

        double length() const {
            return sqrt(length_squared());
        }

        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }

        bool near_zero() const {
            // Return true if vector is close to zero in all dimensions
            auto s = 1e-8;
            return (fabs(e[0]) < s) && fabs(e[1] < s) && fabs(e[2] < 2);
        }

        static vec3 random() {
            return vec3(random_double(), random_double(), random_double());
        }

        static vec3 random(double min, double max) {
            return vec3(random_double(min, max), random_double(min, max), random_double(min, max)); 
        }
};

// vec3 Type aliases
using point3 = vec3; // 3D point
using colour = vec3; // RGB colour

// vec3 Utility Functions

std::ostream& operator<<(std::ostream &out, const vec3 &v) 
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

vec3 operator+(const vec3 &u, const vec3 &v) 
{
    return vec3{u[0] + v[0], u[1] + v[1], u[2] + v[2]};
}

vec3 operator-(const vec3 &u, const vec3 &v) 
{
    return vec3{u[0] - v[0], u[1] - v[1], u[2] - v[2]};
}

vec3 operator*(const vec3 &u, const vec3 &v) 
{
    return vec3{u[0] * v[0], u[1] * v[1], u[2] * v[2]};
}

vec3 operator*(const vec3 &u, double t) 
{
    return vec3{t*u[0], t*u[1], t*u[2]};
}

vec3 operator*(double t, const vec3 &u) 
{
    return u * t;
}

vec3 operator/(const vec3 &u, double t) 
{
    return (1/t) * u;
}

double dot(const vec3 &u, const vec3 &v)
{
    return u[0] * v[0]
         + u[1] * v[1]
         + u[2] * v[2];
}

vec3 cross(const vec3 &u, const vec3 &v)
{
    return vec3{u[1] * v[2] - u[2] * v[1],
                u[2] * v[0] - u[0] * v[2],
                u[0] * v[1] - u[1] * v[0] };
}

vec3 unit_vector(const vec3 &v)
{
    return v / v.length();
}

vec3 random_in_unit_sphere() {
    while (true) {
        // Reject vectors that fall outside unit sphere
        auto p = vec3::random(-1, 1);
        if (p.length_squared() < 1) {
            return p;
        }
    }
}

vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_unit_vector();
    // Invert vector if not facing same direction as surface normal (pointing into sphere)
    if (dot(on_unit_sphere, normal) > 0.0) {
        return on_unit_sphere;
    } else {
        return -on_unit_sphere;
    }
}

vec3 reflect(const vec3& v, const vec3& n) {
    // Explanation:
    // - We want a vector the length of v pointing at a right angle
    //   out from where it hit the surface (easier with a diagram lol).
    // - Note that n is supposed to be the normal vector, and v is the 
    //   ray hitting the surface.
    // - b = dot(-v, n) gives the length from v back to the "surface" (in the direction of n)
    // - Adding 2*b to v gives the desired vector
    return v - 2*dot(v, n)*n;
}

// Note: it is assumed that the surface normal n faces towards the incident ray uv
// Note: it is also assumed that uv and n are unit vectors
vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-uv, n), 1.0); // (flip uv to get a positive result)
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

#endif

/*
int main()
{
    // Testing l/rvalue stuff (and non-const overloads)
    vec3 v = vec3{ 1, 2, 3 } += vec3{1, 2, 3};

    std::cout << v;

    v = vec3{ 1, 2, 3 } + vec3{1, 2, 3};
}
*/