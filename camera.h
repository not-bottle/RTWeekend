#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "arperture.h"
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

    double defocus_radius = 1.0;

    std::shared_ptr<arperture> arp; // Optionally define arpeture using a 1-bit png

    void render(const hittable& world, std::vector<colour>& data, int& scanlines) {
        initialize();
        scanlines = image_height;

        for (int j = 0; j < image_height; ++j) {

            //std::cerr << "Thread " << std::this_thread::get_id() << ": " << "\rScanlines remaining: " << image_height - j << ' ' << std::flush;

            for (int i = 0; i < image_width; ++i) {
                colour pixel_colour(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample) {
                    ray r = get_ray(image_width-i, image_height-j);
                    pixel_colour += ray_colour(r, max_depth, world);
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
        auto viewport_upper_left = centre - (focus_dist * -w) - viewport_u/2 - viewport_v/2;
        // Find the centre of the upper left pixel
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        //auto defocus_radius = focus_dist2 * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

  private:
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
            colour colour_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

            bool scatter = rec.mat->scatter(r, rec, attenuation, scattered);

            // Some materials override the camera to return flat colours for some materials (without generating more rays).
            if (rec.mat->noshade)
                return attenuation; // This is mostly for debugging normals and such
    
            // If we don't scatter, ray is absorbed, return the colour from the emission (colour(0,0,0) unless its a light emitting material)
            if (!scatter)
                return colour_from_emission;
            
            colour colour_from_scatter = attenuation * ray_colour(scattered, depth-1, world);
            
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

    ray get_ray(int i, int j) const {
        // Get a randomly sampled camera ray for the pixel located at i, j, 
        // originating from the camera defocus disk
        auto pixel_centre = pixel00_loc + (i*pixel_delta_u) + (j*pixel_delta_v);
        auto pixel_sample = pixel_centre + pixel_sample_square();

        auto ray_origin = (defocus_radius <= 0) ? centre : defocus_disk_sample();
        // Lens equations
        auto ray_direction = ray_origin - pixel_sample;

        //auto arpl = std::sqrt(ray_direction.x()*ray_direction.x() + ray_direction.y()*ray_direction.y());
        //auto arpd = (ray_origin - centre).length();
        //float fact = 1.0 + ((ray_direction.z()/arpl) * (arpd/focal_length));
        //vec3 arpv = vec3(ray_direction.x()*fact, ray_direction.y()*fact, ray_direction.z());

        point3 lens_centre = centre;
        auto d = focal_length*focus_dist/(focus_dist - focal_length);
        auto focus_plane_centre = lens_centre + d*-w;
        auto focus_plane_normal = w;
        ray centre_ray = ray(pixel_sample, unit_vector(lens_centre - pixel_sample));
        auto denom = dot(centre_ray.direction(), focus_plane_normal);
        auto podotn = dot(focus_plane_centre, focus_plane_normal);
        auto lodotn = dot(centre_ray.origin(), focus_plane_normal);
        auto centre_ray_intersection = (podotn - lodotn)/denom;
        point3 focus_point = centre_ray.at(centre_ray_intersection);
        //std::cerr << "d: " << d << std::endl;
        // std::cerr << "----------------------\n" << pixel_sample.z() << std::endl;
        // std::cerr << lens_centre.z() << std::endl;
        // //std::cerr << centre_ray.orig << std::endl;
        // //std::cerr << centre_ray_intersection << std::endl;
        // //std::cerr << w << std::endl;

        // std::cerr << focus_plane_centre.z() << std::endl;
        // //std::cerr << focus_plane_normal << std::endl;
        // std::cerr << focus_point.z() << std::endl;

        auto ray_time = random_double();

        return ray(ray_origin, unit_vector(focus_point - ray_origin), ray_time);
    }

    point3 defocus_disk_sample() const {
        // Return a random point in the camera defocus disk
        if (!arp) {
            auto p = random_in_unit_disk();
            return centre + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        } else {
            return arp->sample(centre, defocus_disk_u, defocus_disk_v);
        }
    }

    vec3 pixel_sample_square() const {
        // Returns a random point within a pixel (the square surrounding the pixel's centre)

        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (px * pixel_delta_v);
    }
};

#endif