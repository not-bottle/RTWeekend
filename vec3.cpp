#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

int hello = 1;

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
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];
            return *this;
        }

        vec3& operator*=(const double t) {
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;
            return *this;
        }

        vec3& operator/=(const double t) {
            return *this *= 1/t;
        }

        double length() const {
            return sqrt(length_squared());
        }

        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }
};

// vec3 Type aliases
using point3 = vec3; // 3D point
using colour = vec3; // RGB colour
#endif

int main()
{
    vec3 v = vec3(9.2, 7, 303.1);
    vec3 v1 = vec3(0, 0, 0);

    v += v1;

    double d = v[0];

    std::cout << "v[0]="  << v[0] << " d= " << d << '\n';

    v[0] = 7.77;

    int a = 0;
    a += 1;

    int *p = ((int *) malloc(sizeof(int)));
    free(p);

    *p = 0x4010;

    p = ((int *) malloc(sizeof(int)));
    p = ((int *) malloc(sizeof(int)));

    *p = 9;


    malloc(sizeof(int));

    std::cout << "v[0]= " << v[0] << " d= " << d << '\n';
}