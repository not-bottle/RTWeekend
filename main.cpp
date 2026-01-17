#include "rtweekend.h"

#include "bvh.h"
#include "camera.h"
#include "colour.h"
#include "constant_medium.h"
#include "hittable_list.h"
#include "material.h"
#include "quad.h"
#include "sphere.h"
#include "triangle.h"
#include "texture.h"
#include "render.h"

#include <curses.h>
#include <chrono>
#include <fstream>

void load_final_scene(hittable_list& world, camera& cam);
void load_final_scene_motion_blur(hittable_list& world, camera& cam);
void load_checkered_spheres(hittable_list& world, camera& cam);
void load_earth(hittable_list& world, camera& cam);
void load_suzanne_scene(hittable_list& world, camera& cam);
void load_suzanne_normal(hittable_list& world, camera& cam);
void load_teapot(hittable_list& world, camera& cam);
void load_perlin_spheres(hittable_list& world, camera& cam);
void load_quads(hittable_list& world, camera& cam);
void load_simple_light(hittable_list& world, camera& cam);
void load_cornell_box(hittable_list& world, camera& cam);
void load_cornell_smoke(hittable_list& world, camera& cam);
void load_final_scene2(hittable_list& world, camera& cam, int image_width, int samples_per_pixel, int max_depth);
void load_water_scene(hittable_list& world, camera& cam);

const int THREAD_COUNT = 1;

std::string fname = "test.ppm";

int main() {
    // Open image file
    std::ofstream output_file;
    output_file.open(fname);
    if (!output_file.is_open()) {
        std::cerr << "Error opening image file" << std::endl;
        exit(1);
    }

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    clear();

    hittable_list world;
    camera cam;

    load_water_scene(world, cam);

    cam.image_width = 400;
    cam.samples_per_pixel = 32;
    cam.focus_dist = 2.0;
    cam.defocus_radius = 0.03;
    auto object_dist = 10.0;

    cam.focal_length = (cam.focus_dist * object_dist)/(cam.focus_dist + object_dist);

    cam.arp = std::make_shared<arperture>(std::make_shared<rtw_image>("swipe.png"));
    world = hittable_list(std::make_shared<bvh_node>(world));
    render r{THREAD_COUNT};
    auto render_start_time = std::chrono::steady_clock::now();
    r.create_image(output_file, cam, world);
    auto render_finish_time = std::chrono::steady_clock::now();
    auto render_duration = std::chrono::duration_cast<std::chrono::milliseconds>(render_finish_time - render_start_time).count();
    endwin();

    std::cout << "Render time: " << render_duration << "ms" << std::endl;

    exit(0);
}

void load_suzanne_normal(hittable_list& world, camera& cam) 
{
    auto material_normal = std::make_shared<shade_normal>();
    Model model = Model("./test_objects/suzanne.obj");
    mesh_to_hittables(model, world, material_normal, vec3(0.0, 0.0, 0.0));
    world = hittable_list(std::make_shared<bvh_node>(world));

    std::cerr << "World Size: " << world.objects.size() << std::endl;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 32;
    cam.max_depth         = 50;

    cam.vfov     = 90;
    cam.lookfrom = point3(0,0.0,1.5);
    cam.lookat   = point3(0,0,1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 1.0;    
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

void load_water_scene(hittable_list& world, camera& cam)
{
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    //auto ground_material = std::make_shared<image_texture>("sand_03_diff_4k.jpg", 700.0);
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<dielectric>(1.333/1.0)));

    for (int a = -22; a < 22; a++) {
        for (int b = -22; b < 22; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2 + 2.0 * random_double(), b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9 && (center - point3(13, 2, 3)).length() > 3.0) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.0) {
                    // diffuse
                    auto albedo = colour::random() * colour::random();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.03) {
                    // light
                    center = center - point3(0, 0.5, 0);
                    //auto dir = cam.lookat - cam.lookfrom;
                    //center = center + unit_vector(dir)*random_double()*1.5;
                    auto brightness_scale = 3.0 + 12.0*(random_double());
                    auto rscale =  1.5 * (random_double() - 0.5);
                    auto gscale = 1.5 * (random_double() - 0.5);
                    sphere_material = std::make_shared<diffuse_light>(colour((.9 + rscale)*brightness_scale, (.8 + rscale)*brightness_scale, .6));
                    world.add(std::make_shared<sphere>(center, 0.15 + (random_double()-0.5)*0.35, sphere_material));
                } else {
                    // glass
                    // sphere_material = std::make_shared<dielectric>(1.00/1.333); // Air bubble ratio
                    sphere_material = std::make_shared<dielectric>(1.333/1.0); // Water droplet ratio
                    world.add(std::make_shared<sphere>(center, 0.15 - (random_double())*0.35, sphere_material));
                }
            }
        }
    }

    //auto material1 = std::make_shared<dielectric>(1.5/1.333);
    auto material1 = std::make_shared<dielectric>(1.5/1.333);
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(colour(0.8, 0.5, 0.4));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(colour(0.8, 0.5, 0.4), 0.0);
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    auto boundary = std::make_shared<sphere>(point3(0,0,0), 20, std::make_shared<dielectric>(1.5));
    world.add(std::make_shared<constant_medium>(boundary, .00075, colour (.9,.8,.7)));

    cam.background = std::make_shared<image_texture>("kloofendal_48d_partly_cloudy_puresky_4k.png"); 
    cam.background_brightness = 1.5;
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

    auto ground_material = std::make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(ground_material)));

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

    world = hittable_list(std::make_shared<bvh_node>(world));
}

void load_checkered_spheres(hittable_list& world, camera& cam) {
    auto checker = std::make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));

    world.add(std::make_shared<sphere>(point3(0,-10, 0), 10, std::make_shared<lambertian>(checker)));
    world.add(std::make_shared<sphere>(point3(0, 10, 0), 10, std::make_shared<lambertian>(checker)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_earth(hittable_list& world, camera& cam) {
    auto earth_texture = std::make_shared<image_texture>("earthmap.jpg", 10);
    auto earth_surface = std::make_shared<lambertian>(earth_texture);
    world.add(std::make_shared<sphere>(point3(0,0,0), 2, earth_surface));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_suzanne_scene(hittable_list& world, camera& cam) {
    auto material_ground = std::make_shared<lambertian>(colour(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<lambertian>(colour(0.1, 0.2, 0.5));
    auto material_left   = std::make_shared<dielectric>(1.50);
    auto material_bubble = std::make_shared<dielectric>(1.00 / 1.50);
    auto material_right  = std::make_shared<metal>(colour(0.8, 0.6, 0.2), 0.075);
    auto material_right2  = std::make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);

    auto material_earth = std::make_shared<lambertian>(std::make_shared<image_texture>("earthmap.jpg"));

    world.add(std::make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<sphere>(point3( 0.0,  0.0,   -3.0),   1.0, material_center));
    world.add(std::make_shared<sphere>(point3( 1.25,  0.0,    0.75),   0.5, material_bubble));
    world.add(std::make_shared<sphere>(point3(-1.25,  0.0,    0.75),   0.5, material_right));
    world.add(std::make_shared<sphere>(point3( 0.0,  8.0,    -7.0),   6.0, material_right2));

    //Model model = Model("./test_objects/suzanne.obj");
    //mesh_to_hittables(model, world, material_earth, vec3(0.0, 0.0, 0.0));
    //world = hittable_list(std::make_shared<bvh_node>(world));

    world.add(std::make_shared<quad>(point3(-0.5,-0.5, 1), vec3(1, 0, 0), vec3(0, 1, 0), material_earth));

    std::cerr << "World Size: " << world.objects.size() << std::endl;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 64;
    cam.max_depth         = 50;

    cam.vfov     = 90;
    cam.lookfrom = point3(0,0,2.0);
    cam.lookat   = point3(0,0,1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 1.0;
}

void load_teapot(hittable_list& world, camera& cam) {
    //auto material_normal = std::make_shared<metal>(colour(0.6, 0.7, 0.6), 0.0);
    auto material_normal = std::make_shared<dielectric>(1.33);
    //auto material_normal = std::make_shared<shade_normal>();
    Model model = Model("./test_objects/newell_teaset/teapot.obj");
    //model.smoothVertexNormals();
    mesh_to_hittables(model, world, material_normal, vec3(0.0, 0.0, 0.0));
    world = hittable_list(std::make_shared<bvh_node>(world));

    std::cerr << "World Size: " << world.objects.size() << std::endl;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 64;
    cam.max_depth         = 50;

    cam.vfov     = 45;
    cam.lookfrom = point3(5.0,1.5,0.0);
    cam.lookat   = point3(0,1.5,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 1.0;

    cam.background = std::make_shared<image_texture>("772-hdri-skies-com.png"); 
    cam.background_brightness = 2.0;
}

void load_perlin_spheres(hittable_list& world, camera& cam) {

    auto pertext = std::make_shared<noise_texture>(4);
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(pertext)));
    world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(pertext)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_quads(hittable_list& world, camera& cam) {
    // Materials
    auto left_red     = std::make_shared<lambertian>(colour(1.0, 0.2, 0.2));
    auto back_green   = std::make_shared<lambertian>(colour(0.2, 1.0, 0.2));
    auto right_blue   = std::make_shared<lambertian>(colour(0.2, 0.2, 1.0));
    auto upper_orange = std::make_shared<lambertian>(colour(1.0, 0.5, 0.0));
    auto lower_teal   = std::make_shared<lambertian>(colour(0.2, 0.8, 0.8));

    // Quads
    world.add(std::make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(std::make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(std::make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(std::make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(std::make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_simple_light(hittable_list& world, camera& cam) {
    auto pertext = std::make_shared<noise_texture>(4);
    world.add(std::make_shared<sphere>(point3(0,-1001,0), 1000, std::make_shared<lambertian>(pertext)));
    //world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(pertext)));

    //Model model = Model("./test_objects/suzanne.obj");
    //mesh_to_hittables(model, world, std::make_shared<dielectric>(1.5), vec3(0.0, 0.0, 0.0));
    //world = hittable_list(std::make_shared<bvh_node>(world));

    world.add(std::make_shared<sphere>(point3(0, 0, 0), 1.0, std::make_shared<dielectric>(1.5)));

    // Note that light value is set "brighter" than colour(1, 1, 1). This allows it to light things brightly.
    // (Recall that calculations are made using attenuation * colour from scatter + emission, so multiplying by 1 won't do much)
    auto difflight1 = std::make_shared<diffuse_light>(colour(7,7,7));
    auto difflight2 = std::make_shared<diffuse_light>(colour(0.3,0.5,10));
    world.add(std::make_shared<sphere>(point3(0,4.0,0), 2.0, difflight1));
    //world.add(std::make_shared<quad>(point3(1.0,.5,-1), vec3(1.5,0,0), vec3(0,1.5,0), difflight2));
    for (int i = -40; i < 40; i += 4)
    {
        for (int j = 0; j < 40; j += 5)
        {
            auto c = std::make_shared<diffuse_light>(colour::random() * colour::random());
            world.add(std::make_shared<sphere>(point3(i+random_double()*8.0,j+random_double()*8.0,-80.0), 0.5, c));
        }
    }

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 1000;
    cam.max_depth         = 50;
    cam.background        = std::make_shared<solid_colour>(colour(0,0,0));

    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,8);
    cam.lookat   = point3(0,.25,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_cornell_box(hittable_list& world, camera& cam) {
    auto red   = std::make_shared<lambertian>(colour(.65, .05, .05));
    auto white = std::make_shared<lambertian>(colour(.73, .73, .73));
    auto green = std::make_shared<lambertian>(colour(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(colour(15, 15, 15));

    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(std::make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = std::make_shared<solid_colour>(colour(0,0,0));

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_cornell_smoke(hittable_list& world, camera& cam) {
    auto red   = std::make_shared<lambertian>(colour(.65, .05, .05));
    auto white = std::make_shared<lambertian>(colour(.73, .73, .73));
    auto green = std::make_shared<lambertian>(colour(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(colour(7, 7, 7));

    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(std::make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    world.add(std::make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));

    std::shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130,0,65));

    world.add(std::make_shared<constant_medium>(box1, 0.01, colour(0,0,0)));
    world.add(std::make_shared<constant_medium>(box2, 0.01, colour(1,1,1)));

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = std::make_shared<solid_colour>(colour(0, 0, 0));

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}

void load_final_scene2(hittable_list& world, camera& cam, int image_width, int samples_per_pixel, int max_depth) {
    hittable_list boxes1;
    auto ground = std::make_shared<lambertian>(colour (0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    world.add(std::make_shared<bvh_node>(boxes1));

    auto light = std::make_shared<diffuse_light>(colour (7, 7, 7));
    world.add(std::make_shared<quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto sphere_material = std::make_shared<lambertian>(colour (0.7, 0.3, 0.1));
    world.add(std::make_shared<sphere>(center1, center2, 50, sphere_material));

    world.add(std::make_shared<sphere>(point3(260, 150, 45), 50, std::make_shared<dielectric>(1.5)));
    world.add(std::make_shared<sphere>(
        point3(0, 150, 145), 50, std::make_shared<metal>(colour (0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = std::make_shared<sphere>(point3(360,150,145), 70, std::make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(std::make_shared<constant_medium>(boundary, 0.2, colour (0.2, 0.4, 0.9)));
    boundary = std::make_shared<sphere>(point3(0,0,0), 5000, std::make_shared<dielectric>(1.5));
    world.add(std::make_shared<constant_medium>(boundary, .0001, colour (1,1,1)));

    auto emat = std::make_shared<lambertian>(std::make_shared<image_texture>("earthmap.jpg"));
    world.add(std::make_shared<sphere>(point3(400,200,400), 100, emat));
    auto pertext = std::make_shared<noise_texture>(0.2);
    world.add(std::make_shared<sphere>(point3(220,280,300), 80, std::make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = std::make_shared<lambertian>(colour (.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(std::make_shared<sphere>(point3::random(0,165), 10, white));
    }

    world.add(std::make_shared<translate>(
        std::make_shared<rotate_y>(
            std::make_shared<bvh_node>(boxes2), 15),
            vec3(-100,270,395)
        )
    );

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width;
    cam.samples_per_pixel = samples_per_pixel;
    cam.max_depth         = max_depth;
    cam.background        = std::make_shared<solid_colour>(colour(0, 0, 0));

    cam.vfov     = 40;
    cam.lookfrom = point3(478, 278, -600);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}
