#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "colour.h"
#include "hittable.h"
#include "material.h"
#include "texture.h"

#include <iostream>
#include <thread>

class camera {
  public:
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int image_height; // Rendered image height
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10; // Maximum number of times rays are allowed to bounce
    int total_stratified_samples = 10;

    double vfov = 90; // The vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, -1); // Point camera is looking from
    point3 lookat   = point3(0, 0, 0); // Point camera is looking at
    vec3 vup = vec3(0, 1, 0); // The relative "up" direction for the camera

    double defocus_angle = 0; // Variation angle of rays originating from disk
    //                           (Think of it like a cone) 
    double focus_dist = 1.0; // Distance from lookfrom point to plane (of perfect focus)
    double focal_length = 10.0;
    std::shared_ptr<texture> background = std::make_shared<sky_gradient>();
    double background_brightness = 1.0;

    double defocus_radius = 1.0; // Note: This doesn't do anything since the original defocus blur is being used

    void render(const hittable& world, std::vector<colour>& data, int& scanlines) {
        initialize();
        scanlines = image_height;

        for (int j = 0; j < image_height; ++j) {

            //std::cerr << "Thread " << std::this_thread::get_id() << ": " << "\rScanlines remaining: " << image_height - j << ' ' << std::flush;

            for (int i = 0; i < image_width; ++i) {
                colour pixel_colour(0, 0, 0);
                for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_colour += ray_colour(r, max_depth, world);
                    }
                }
                data.push_back(pixel_colour);
            }
            scanlines -= 1;
        }
        scanlines = 0; // Ensure this condition is met for thread polling
        //std::cerr << "Thread " << std::this_thread::get_id() << ": " << "\nDone.\n";
    }

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height; // Ensure height is at least 1

        sqrt_spp = int(std::sqrt(samples_per_pixel));
        total_stratified_samples = sqrt_spp * sqrt_spp;
        recip_sqrt_spp = 1.0 / sqrt_spp;

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

  private:
    int sqrt_spp; // Square root of number of samples per pixel
    double recip_sqrt_spp; // 1 / sqrt_spp
    point3 centre; // Camera centre
    point3 pixel00_loc; // Localtion of pixel 0,0
    vec3 pixel_delta_u; // Offset to pixel to the right
    vec3 pixel_delta_v; // Offset to pixel below
    vec3 u, v, w; // Camera basis vectors (orthonormal)
    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius
  
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
            double pdf_value;
            colour colour_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

            bool scatter = rec.mat->scatter(r, rec, attenuation, scattered, pdf_value);

            // Some materials override the camera to return flat colours for some materials (without generating more rays).
            if (rec.mat->noshade)
                return attenuation; // This is mostly for debugging normals and such
    
            // If we don't scatter, ray is absorbed, return the colour from the emission (colour(0,0,0) unless its a light emitting material)
            if (!scatter)
                return colour_from_emission;
            
            double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);
            pdf_value = scattering_pdf;
            colour colour_from_scatter = (attenuation * scattering_pdf * ray_colour(scattered, depth-1, world)) / pdf_value;
            
            return colour_from_emission + colour_from_scatter;

        }
    
        // If ray hits nothing return background colour (using a skysphere texture)
        vec3 unit_dir = unit_vector(r.dir);
        
        double skysphere_radius = 13067000.0;
        point3 skysphere_origin = vec3();
        vec3 oc = r.origin() - skysphere_origin;
        auto a = r.direction().length_squared();
        auto half_b = dot(r.direction(), oc);
        auto c = oc.length_squared() - skysphere_radius*skysphere_radius;
        auto discriminant = half_b*half_b - a*c;
        if (discriminant < 0) return vec3(0, 1, 1);
        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (root < 0.001) {
            root = (-half_b + sqrtd) / a;
        }

        vec3 hit_normal = (r.at(root) - skysphere_origin) / skysphere_radius;

        double phi = std::atan2(-hit_normal.z(), hit_normal.x()) + pi;
        double theta = std::acos(-hit_normal.y());
        return background_brightness * background->value(phi / (2*pi), theta / pi, unit_dir);
    }

    ray get_ray(int i, int j, int s_i, int s_j) const {
        // Construct a camera ray originating from the defocus disk and directed
        // at a randomly sampled point around the pixel location i, j for stratified
        // samplesquare s_i, s_j.
        auto offset = sample_square_stratified(s_i, s_j);
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? centre : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square_stratified(int s_i, int s_j) const {
        // Return the vector to a random point in the square sub-pixel
        // specified by grid indices s_i and s_j, for an idealized
        // unit square pixel [-.5, -.5] to [+.5, +.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return vec3(px, py, 0);
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
