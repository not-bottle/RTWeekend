#if !defined(VEC3_H)
#define VEC3_H

#include "matrix.h"

/* Generic class to hold a 3D vector. Defines common operations. */
class vec3 {
public:
    std::array<double, 3> e {};

    vec3();
    vec3(const matrix<double, 1, 3> &m);
    vec3(double e0, double e1, double e2); 

    double x() const; 
    double y() const; 
    double z() const; 
    
    double operator[](int i) const;
    double& operator[](int i) ;
    
    double length() const;
    double length_squared() const;
    
    bool near_zero() const;

    static vec3 random();
    static vec3 random(double min, double max);
};

// vec3 Utility Functions
std::ostream& operator<<(std::ostream &out, const vec3 &v); 

bool operator==(const vec3 &a, const vec3 &b);

vec3 operator-(const vec3 &a);

vec3& operator+=(vec3 &a, const vec3 &b);

vec3& operator-=(vec3 &a, const vec3 &b);

vec3& operator*=(vec3 &m, double c);

vec3& operator/=(vec3 &m, double c);

vec3 operator+(const vec3& a, 
                  const vec3& b);

vec3 operator-(const vec3& a, 
                  const vec3& b);

vec3 operator*(const vec3& a, 
                  const vec3& b);

vec3 operator*(const vec3& m, double c);

vec3 operator*(double c, const vec3& m);

vec3 operator/(const vec3& m, double c);

double dot(const vec3 &u, const vec3 &v);

vec3 cross(const vec3 &u, const vec3 &v);

vec3 unit_vector(const vec3 &v);

vec3 reflect(const vec3 &v, const vec3 &n);

vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat);
    
#include "vec3.cpp"

#endif
