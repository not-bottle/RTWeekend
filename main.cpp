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
    hittable_list lights;
    camera cam;

    load_cornell_box(world, lights, cam);

    cam.samples_per_pixel = 100;
    render r{THREAD_COUNT};
    auto render_start_time = std::chrono::steady_clock::now();
    r.create_image(output_file, cam, world, lights);
    auto render_finish_time = std::chrono::steady_clock::now();
    auto render_duration = std::chrono::duration_cast<std::chrono::milliseconds>(render_finish_time - render_start_time).count();
    endwin();

    std::cout << "Render time: " << render_duration << "ms" << std::endl;

    exit(0);
}

