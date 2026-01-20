#include "random_help.h"

#include <cmath>
#include <iostream>

/* Constructors */
vec3::vec3() {}

vec3::vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

vec3::vec3(const matrix<double, 1, 3> &m)
{
    e[0] = m(0, 0);
    e[1] = m(0, 1);
    e[2] = m(0, 2);
}

/* Class Functions */
double vec3::x() const { return e[0]; }
double vec3::y() const { return e[1]; }
double vec3::z() const { return e[2]; }

double vec3::operator[](int i) const { return e[i]; }
double& vec3::operator[](int i) { return e[i]; }

double vec3::length() const 
{
    return sqrt(length_squared());
}

double vec3::length_squared() const 
{
    return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
}

bool vec3::near_zero() const 
{
    // Return true if vector is close to zero in all dimensions
    auto s = 1e-8;
    return (fabs(e[0]) < s) && fabs(e[1] < s) && fabs(e[2] < s);
}

/* Static Member Functions */
vec3 vec3::random() 
{
    return vec3(random_double(), random_double(), random_double());
}

vec3 vec3::random(double min, double max) 
{
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max)); 
}

/* Operator Overloads */
std::ostream& operator<<(std::ostream &out, const vec3 &v) 
{
    return out << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
}

bool operator==(const vec3 &a, const vec3 &b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

vec3 operator-(const vec3 &a)
{
    return vec3(-a.e[0], -a.e[1], -a.e[2]);
}

vec3& operator+=(vec3 &a, const vec3 &b)
{
    a.e[0] += b[0];
    a.e[1] += b[1];
    a.e[2] += b[2];
    return a;
}

vec3& operator-=(vec3 &a, const vec3 &b)
{
    return a += (-b);
}

vec3& operator*=(vec3 &m, double c)
{
    m[0] *= c;
    m[1] *= c;
    m[2] *= c;
    return m;
}

vec3& operator/=(vec3 &m, double c)
{
    return m *= (1.0/c);
}

vec3 operator+(const vec3& a, 
                  const vec3& b)
{
    return vec3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

vec3 operator-(const vec3& a, 
                  const vec3& b)
{
    return a + (-b);
}

vec3 operator*(const vec3& a, 
                  const vec3& b)
{
    return vec3(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

vec3 operator*(const vec3& m, double c)
{
    return vec3(m[0] * c, m[1] * c, m[2] * c);
}

vec3 operator*(double c, const vec3& m)
{
    return m * c;
}

vec3 operator/(const vec3& m, double c)
{
    return m * (1.0/c);
}

/* Non-Class Functions */
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

vec3 reflect(const vec3 &v, const vec3 &n) 
{
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
vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat) 
{
    auto cos_theta = fmin(dot(-uv, n), 1.0); // (flip uv to get a positive result)
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
