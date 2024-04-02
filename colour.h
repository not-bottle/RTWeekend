#ifndef COLOUR_H
#define COLOUR_H

#include "rtweekend.h"
#include <iostream>

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

    // Write the translated [0,255] value of each color component.
    static const interval intensity(0.000, 0.999);
    out << static_cast<int>(255.999 * intensity.clamp(r)) << ' '
        << static_cast<int>(255.999 * intensity.clamp(g)) << ' '
        << static_cast<int>(255.999 * intensity.clamp(b)) << '\n';
}

#endif