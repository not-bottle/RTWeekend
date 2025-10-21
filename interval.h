#ifndef INTERVAL_H
#define INTERVAL_H

#include "rtweekend.h"

class interval {
    public:
        double min, max;

        interval() : min(+infinity), max(-infinity) {} // Default interval is empty

        interval(double _min, double _max) : min(_min), max(_max) {}

        interval (double a, double b, double c) 
        {
            min = a <= b ? 
                    a <= c ? a : c
                :   b <= c ? b : c;
            max = a >= b ? 
                    a >= c ? a : c
                :   b >= c ? b : c;
        }

        interval(interval a, interval b) 
        {
            min = a.min <= b.min ? a.min : b.min;
            max = a.max >= b.max ? a.max : b.max;
        }

        friend std::ostream& operator<<(std::ostream& ostream, interval i);

        bool contains(double x) const {
            return min <= x && x <= max;
        }

        bool surrounds(double x) const {
            return min < x && x < max;
        }

        double clamp(double x) const {
            if (x < min) return min;
            if (x > max) return max;
            return x;
        }

        double size() const { return max - min; }

        interval expand(double delta) const {
            auto padding = delta/2;
            return interval(min - padding, max + padding);
        }

        static const interval empty, universe;
};

std::ostream& operator<<(std::ostream& ostream, interval i)
{ 
    ostream << "(" << i.min << ", " << i.max << ")"; 
    return ostream;
}

const interval interval::empty {+infinity, -infinity};
const interval interval::universe {-infinity, +infinity}; 

#endif