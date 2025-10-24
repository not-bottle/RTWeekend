#ifndef RENDER_H
#define RENDER_H

#include "vec3.h"
#include "camera.h"
#include "hittable_list.h"

#include <iostream>
#include <thread>
#include <vector>
#include <functional>

class render
{
    public:
        render(int requested_threadcount)
        {
            int hardware_threads = std::thread::hardware_concurrency();
            num_threads = requested_threadcount >= hardware_threads ? hardware_threads : requested_threadcount;
            if (num_threads <= 0) num_threads = 1;
        }

        void create_image(std::ostream& outstream, camera cam, hittable_list& hittables) {
            int total_samples = cam.samples_per_pixel;
            // Define samples per-thread in the camera object
            cam.samples_per_pixel = cam.samples_per_pixel / num_threads;
            int remaining_samples = total_samples - (cam.samples_per_pixel * num_threads);
            

            // Create threads
            std::vector<std::vector<colour>> output;
            std::vector<std::thread> threads;
            
            for (int i = 0; i < num_threads - 1; i++)
            {
                std::vector<colour> t_storage{};
                output.push_back(t_storage);
                std::thread thread = std::thread(render_thread, std::ref(cam), std::ref(hittables), std::ref(t_storage));
                threads.push_back(thread);
                std::cerr << "Created new render thread " << thread.get_id() << " for " << cam.samples_per_pixel << " samples." << std::endl << std::flush;
            }
            
            // Render the rest of the samples
            std::vector<colour> image_data{};
            camera& cam_final_render{cam};
            if (remaining_samples != cam.samples_per_pixel)
            {
                cam_final_render = camera(cam); // Copy camera so that samples_per_pixel can be changed
                cam_final_render.samples_per_pixel = remaining_samples;
            }
            render_thread(cam_final_render, hittables, image_data);

            // Join threads
            std::for_each(threads.begin(),threads.end(),
                std::mem_fn(&std::thread::join));
            for (std::thread& t : threads) {
                t.join();
                std::cerr << "Thread " << t.get_id() << " joined." << std::endl << std::flush;
            }

            // Average the outputs
            for (std::vector<colour>& thread_output : output) {
                std::transform(thread_output.begin(), thread_output.end(), image_data, 
                    [](const colour& a, const colour& b){ return a + b; });
            }
            std::transform(image_data.begin(), image_data.end(), image_data.begin(), 
                [total_samples](const colour& a){ return a/total_samples; });

            // Output the Image
            for (colour colour : image_data) {

            }

            outstream << "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

            for (int j = 0; j < cam.image_height; ++j) {
                for (int i = 0; i < cam.image_width; ++i) {
                    colour pixel_colour(0, 0, 0);
                    write_colour(outstream, pixel_colour, total_samples);
                }
            }
        }

    private:
        int num_threads;

        void render_thread(camera& cam, hittable_list& hittables, std::vector<colour>& pixel_data) {
            pixel_data = cam.render(hittables);
        }

};


#endif // RENDER_H
