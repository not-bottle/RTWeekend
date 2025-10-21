#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "colour.h"
#include "hittable.h"
#include "material.h"

#include <iostream>

class camera {
  public:
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10; // Maximum number of times rays are allowed to bounce

    double vfov = 90; // The vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, -1); // Point camera is looking from
    point3 lookat   = point3(0, 0, 0); // Point camera is looking at
    vec3 vup = vec3(0, 1, 0); // The relative "up" direction for the camera

    double defocus_angle = 0; // Variation angle of rays originating from disk
    //                           (Think of it like a cone) 
    double focus_dist = 10; // Distance from lookfrom point to plane (of perfect focus)

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; ++j) {

            std::cerr << "\rScanlines remaining: " << image_height - j << ' ' << std::flush;

            for (int i = 0; i < image_width; ++i) {
                colour pixel_colour(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample) {
                    ray r = get_ray(i, j);
                    pixel_colour += ray_colour(r, max_depth, world);
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
    vec3 u, v, w; // Camera basis vectors (orthonormal)
    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius


    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height; // Ensure height is at least 1

        // Camera

        // Define the origin (rays originate from here)
        centre = lookfrom;

        // Viewport dimensions

        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta/2.0);

        auto viewport_height = 2.0 * h * focus_dist;
        // We don't use aspect_ratio to calculate viewport_width as integer rounding may have changed the actual ratio
        auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // Calculate the u,v,w camera orthonormal basis vectors
        w = unit_vector(lookfrom - lookat); // (This will be pointing _opposite_ lookat, as we use -w as the camera direction)
        u = unit_vector(cross(vup, w));
        v = cross(w, u);


        // Define two vectors that travel along the width (u) and height (v) of the viewport
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v; // (Negative as we start at at top left corner)

        // Calculate the distance between pixels by dividing the viewport by image dimensions
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Find the upper left corner of the viewport
        auto viewport_upper_left = centre - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        // Find the centre of the upper left pixel
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }


    colour ray_colour(const ray& r, int depth, const hittable& world) const
    {
        hit_record rec;
        
        // When bounce limit is exceeded return no colour (no more light)
        if (depth <= 0)
            return colour(0.0, 0.0, 0.0);

        // world is a hittable list of all objects
        if (world.hit(r, interval(0.001, infinity), rec)) {
            // Note: 0.001 to infinity is used to avoid floating point errors giving hit coordinates within
            // the object. This leads to "shadow acne" - darker spots that occur due to rays hitting an object
            // multiple times from within the surface.
            
            ray scattered;
            colour attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_colour(scattered, depth-1, world);
            
            // If ray is absorbed, return no colour
            return colour(0, 0, 0);

        }
    
        // Sky
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*colour(1.0, 1.0, 1.0) + a*colour(0.5, 0.7, 1.0);
    }

    ray get_ray(int i, int j) const {
        // Get a randomly sampled camera ray for the pixel located at i, j, 
        // originating from the camera defocus disk
        auto pixel_centre = pixel00_loc + (i*pixel_delta_u) + (j*pixel_delta_v);
        auto pixel_sample = pixel_centre + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? centre : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    point3 defocus_disk_sample() const {
        // Return a random point in the camera defocus disk
        auto p = random_in_unit_disk();
        return centre + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    vec3 pixel_sample_square() const {
        // Returns a random point within a pixel (the square surrounding the pixel's centre)

        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (px * pixel_delta_v);
    }
};

#endif