#include "rtweekend.h"

#include "camera.h"
#include "colour.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"

#include <chrono>

void load_final_scene(hittable_list& world, camera& cam);
void load_final_scene_motion_blur(hittable_list& world, camera& cam);

int main() {
    hittable_list world;
    camera cam;
    auto material_normal = std::make_shared<shade_normal>();
    //auto material_ground = std::make_shared<lambertian>(colour(0.8, 0.8, 0.0));
    //auto material_center = std::make_shared<lambertian>(colour(0.1, 0.2, 0.5));
    //auto material_left   = std::make_shared<dielectric>(1.50);
    //auto material_bubble = std::make_shared<dielectric>(1.00 / 1.50);
    //auto material_right  = std::make_shared<metal>(colour(0.8, 0.6, 0.2), 0.0);

    //world.add(std::make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    //world.add(std::make_shared<sphere>(point3( -1.5,    0.0, 0.5),   0.5, material_center));
    //world.add(std::make_shared<sphere>(point3(0.0, 1.0, 0.0),   0.5, material_center));
    //world.add(std::make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.4, material_bubble));
    //world.add(std::make_shared<sphere>(point3( 2.0, 0.0, 0.5),   0.5, material_right));
    //world.add(std::make_shared<sphere>(point3( 0.0, 0.0, 3.0),   0.5, material_right));

    Model model = Model("./test_objects/suzanne.obj");
    mesh_to_hittables(model, world, material_normal);

    std::cerr << "World Size: " << world.objects.size() << std::endl;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 32;
    cam.max_depth         = 1;

    cam.vfov     = 90;
    cam.lookfrom = point3(0,0,1.5);
    cam.lookat   = point3(0,0,1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 1.0;

    //load_final_scene_motion_blur(world, cam);
    //cam.image_width = 400;
    //cam.samples_per_pixel = 32;


    auto render_start_time = std::chrono::steady_clock::now();
    cam.render(world);
    auto render_finish_time = std::chrono::steady_clock::now();
    auto render_duration = std::chrono::duration_cast<std::chrono::milliseconds>(render_finish_time - render_start_time).count();
    std::cerr << "Render time: " << render_duration << "ms" << std::endl;
}

void load_final_scene(hittable_list& world, camera& cam)
{
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    auto ground_material = std::make_shared<lambertian>(colour(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = colour::random() * colour::random();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = colour::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
}

void load_final_scene_motion_blur(hittable_list& world, camera& cam)
{
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    auto ground_material = std::make_shared<lambertian>(colour(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = colour::random() * colour::random();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    world.add(std::make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = colour::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
}
