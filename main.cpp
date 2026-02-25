#include "rtweekend.h"

#include "bvh.h"
#include "camera.h"
#include "colour.h"
#include "hittable_list.h"
#include "render.h"
#include "scenes.h"

#include <curses.h>
#include <chrono>
#include <fstream>

const int THREAD_COUNT = 10;

int main(int argc, char* argv[]) {
    
    // Initialize scene objects
    hittable_list world;
    hittable_list lights;
    camera cam;

    std::cerr << "Max world size: " << world.objects.max_size() << std::endl;

    std::string usage = "Usage: ./run.sh <scene_name> <optional args>";

    // Process command line args
    if (argc <= 1) {
        std::cout << usage << std::endl;
        exit(0); 
    }
    
    std::string fname = "image.ppm";

    std::stringstream scenearg { argv[1] };
    int scene_code { 0 };
    if (!(scenearg >> scene_code)) {
        std::cout << "Scene code must be an integer." << std::endl;
        std::cout << usage << std::endl;
        exit(0);
    }

    switch (scene_code) 
    {
        case 0:
            load_cornell_box(world, lights, cam);
            break;
        case 1:
            load_suzanne_normal(world, lights, cam);
            break;
        default:
            std::cout << usage << std::endl;
            exit(0);
    }

    int spp { 0 };
    for (int i = 2; i < argc; i++) {
        std::string argstr { argv[i] };
        if (argstr.compare(0, 2, "-o", 0, 2) == 0) {
            fname = argstr.substr(2, argstr.size() - 2);
            std::cerr << fname << std::endl;
        } else if (argstr.compare(0, 4, "-spp", 0, 4) == 0) {
            std::stringstream intstr { argstr.substr(4, argstr.size() - 4) };
            if (!(intstr >> spp) || spp == 0) {
                std::cout << "Samples per pixel argument must be a non-zero integer." << std::endl;
                std::cout << usage << std::endl;
                exit(0);
            }
            std::cerr << spp << std::endl;
        } else {
            std::cout << "Incorrect args supplied." << std::endl;
            std::cout << usage << std::endl;
            exit(0);
        }
    }

    if (spp > 0)
        cam.samples_per_pixel = spp;

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

    render r{THREAD_COUNT};
    hittable_list world_bvh;
    world_bvh.add(std::make_shared<bvh_node>(world));
    auto render_start_time = std::chrono::steady_clock::now();
    r.create_image(output_file, cam, world_bvh, lights);
    auto render_finish_time = std::chrono::steady_clock::now();
    auto render_duration = std::chrono::duration_cast<std::chrono::milliseconds>(render_finish_time - render_start_time).count();
    endwin();

    std::cout << "Render time: " << render_duration << "ms" << std::endl;

    exit(0);
}

