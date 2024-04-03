#include "rtweekend.h"

#include "camera.h"
#include "colour.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

int main()
{
    // Define World

    hittable_list world;

    auto material_ground = std::make_shared<lambertian>(colour(0.8, 0.8, 0.0));
    auto material_centre = std::make_shared<lambertian>(colour(0.7, 0.3, 0.3));
    auto material_left = std::make_shared<metal>(colour(0.8, 0.8, 0.8), 0.3);
    auto material_right = std::make_shared<metal>(colour(0.8, 0.6, 0.2), 1.0);

    world.add(std::make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_centre));
    world.add(std::make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(std::make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    // Set up camera

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    // Render

    cam.render(world);
}
