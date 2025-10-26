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
            cam.initialize();
            int total_samples = cam.samples_per_pixel;
            // Define samples per-thread in the camera object
            cam.samples_per_pixel = cam.samples_per_pixel / num_threads;
            int remaining_samples = total_samples - (cam.samples_per_pixel * (num_threads - 1));
            
            // Calculate number of threads to use
            if (cam.samples_per_pixel <= 0) num_threads = 0;
            std::cerr << "Num threads: " << num_threads - 1 << std::endl;
            int threads_vector_size = (num_threads - 1) > 0 ? num_threads - 1 : 0;
            std::vector<std::vector<colour>> output(threads_vector_size);
            std::vector<std::thread> threads(threads_vector_size);

            // Create threads
            for (int i = 0; i < num_threads - 1; i++)
            {
                output[i] = std::vector<colour>();
                threads[i] = std::thread(&render::render_thread, this, std::ref(cam), std::ref(hittables), std::ref(output[i]));
                std::cerr << "Created new render thread " << threads[i].get_id() << " for " << cam.samples_per_pixel << " samples." << std::endl;
            }
            
            // Render the rest of the samples
            std::vector<colour> image_data{};
            cam.samples_per_pixel = remaining_samples;
            render_thread(cam, hittables, image_data);

            // Join threads
            for (std::thread& t : threads) {
                t.join();
                std::cerr << "Thread joined." << std::endl;
            }

            // Average the outputs
            for (std::vector<colour>& thread_output : output) {
                std::transform(thread_output.begin(), thread_output.end(), image_data.begin(), image_data.begin(), 
                    [](const colour& a, const colour& b){ return a + b; });
            }

            // Output the Image
            outstream << "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

            for (colour c : image_data) {
                write_colour(outstream, c, total_samples);
            }
        }

    private:
        int num_threads;

        void render_thread(camera cam, hittable_list& hittables, std::vector<colour>& pixel_data) {
            cam.render(hittables, pixel_data);
        }

};


#endif // RENDER_H
