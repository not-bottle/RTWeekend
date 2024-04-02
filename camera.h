#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "colour.h"
#include "hittable.h"

#include <iostream>

class camera {
  public:
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; ++j) {

            std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

            for (int i = 0; i < image_width; ++i) {
                colour pixel_colour(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample) {
                    ray r = get_ray(i, j);
                    pixel_colour += ray_colour(r, world);
                }

                write_colour(std::cout, pixel_colour, samples_per_pixel);
            }
        }
        std::cerr << "\nDone.\n";
    }

  private:
    int image_height; // Rendered image height
    point3 centre; // Camera centre
    point3 pixel00_loc; // Localtion of pixel 0,0
    vec3 pixel_delta_u; // Offset to pixel to the right
    vec3 pixel_delta_v; // Offset to pixel below


    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height; // Ensure height is at least 1

        // Camera

        // Define the origin (rays originate from here)
        centre = point3(0, 0, 0);

        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        // We don't use aspect_ratio to calculate viewport_width as integer rounding may have changed the actual ratio
        auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // Define two vectors that travel along the width (u) and height (v) of the viewport
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // Calculate the distance between pixels by dividing the viewport by image dimensions
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Find the upper left corner of the viewport
        auto viewport_upper_left = centre - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
        // Find the centre of the upper left pixel
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }


    colour ray_colour(const ray& r, const hittable& world) const
    {
        hit_record rec;
        
        // world is a hittable list of all objects
        if (world.hit(r, interval(0, infinity), rec)) {
            vec3 direction = random_on_hemisphere(rec.normal);
            // Cast a new ray from the hit point in a random direction away from the sphere, calculate the colour and use 50%
            return 0.5 * ray_colour(ray(rec.p, direction), world);
        }
    
        // Sky
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*colour(1.0, 1.0, 1.0) + a*colour(0.5, 0.7, 1.0);
    }

    ray get_ray(int i, int j) const {
        // Get a randomly sampled camera ray for the pixel located at i, j
        auto pixel_centre = pixel00_loc + (i*pixel_delta_u) + (j*pixel_delta_v);
        auto pixel_sample = pixel_centre + pixel_sample_square();

        auto ray_origin = centre;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 pixel_sample_square() const {
        // Returns a random point within a pixel (or the square surrounding the pixel's centre)

        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (px * pixel_delta_v);
    }
};

#endif