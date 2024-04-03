#ifndef COLOUR_H
#define COLOUR_H

#include "rtweekend.h"
#include <iostream>

/* 
   This function is used to account for image viewers assuming that an
   image has been gamma corrected, or that the colours are in "gamma space".
   The transform here is to square the value to go from gamma space to linear space,
   and to use square root to return from gamma space to linear space.
*/
inline double linear_to_gamma(double linear_component) {
    return sqrt(linear_component);
}

/* Writes colour to output stream in PPM format. */
void write_colour(std::ostream &out, colour pixel_colour, int samples_per_pixel) {
    auto r = pixel_colour.x();
    auto g = pixel_colour.y();
    auto b = pixel_colour.z();

    // Divide the colour by the number of samples
    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    // Apply the linear to gamma transform
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Write the translated [0,255] value of each color component.
    static const interval intensity(0.000, 0.999);
    out << static_cast<int>(255.999 * intensity.clamp(r)) << ' '
        << static_cast<int>(255.999 * intensity.clamp(g)) << ' '
        << static_cast<int>(255.999 * intensity.clamp(b)) << '\n';
}

#endif