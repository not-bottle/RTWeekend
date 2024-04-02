#include "vec3.h"
#include "colour.h"

#include <iostream>

int CoolAdd(int x, int y);

int CoolSubtract(int x, int y)
{
    return CoolAdd(x, -y);
}

int CoolAdd(int x, int y)
{
    return x + y;
}

int main() 
{

    int a[10];
    a[7] = 124567;
    int *p = a;

    int x = 1;
    int* p = &x;

    int**p2 = &p;


    std::cout << a[7] << '\n';
    std::cout << *(p + 7) << '\n';
    return 0;
}

