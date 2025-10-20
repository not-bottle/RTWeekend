#ifndef VEC2_H
#define VEC2_H

class vec2 {
    public :
        double e[2];

        vec2() : e{0, 0} {}
        vec2(double e1, double e2) : e{e1, e2} {}

        double operator[](int i) const { return e[i]; }
        double& operator[](int i) { return e[i]; }; 
};

#endif // VEC2_H
