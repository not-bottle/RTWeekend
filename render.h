#ifndef RENDER_H
#define RENDER_H

#include "vec3.h"
#include "camera.h"
#include "hittable_list.h"

#include <curses.h>
#include <sstream>
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
            int total_samples = cam.total_stratified_samples;
            // Define samples per-thread in the camera object
            cam.samples_per_pixel = total_samples / num_threads;
            int remaining_samples = total_samples - (cam.samples_per_pixel * (num_threads - 1));

            std::ostringstream sstream;
            
            // Calculate number of threads to use
            if (cam.samples_per_pixel <= 0) num_threads = 0;
            sstream << "Num threads: " << num_threads << std::endl;
            addstr(sstream.str().c_str());
            sstream.str("");
            int threads_vector_size = num_threads > 0 ? num_threads : 1;
            std::vector<std::vector<colour>> toutput(threads_vector_size - 1);
            std::vector<std::thread> threads(threads_vector_size);
            std::vector<int> tscanlines(threads_vector_size, -1);

            // Create threads
            for (int i = 0; i < num_threads - 1; i++)
            {
                toutput[i] = std::vector<colour>();
                threads[i] = std::thread(&render::render_thread, this, std::ref(cam), std::ref(hittables), std::ref(toutput[i]), std::ref(tscanlines[i]));
                sstream << "Created new render thread " << threads[i].get_id() << " for " << cam.samples_per_pixel << " samples." << std::endl;
                addstr(sstream.str().c_str());
                sstream.str("");
            }

            refresh();

            std::vector<colour> image_data{}; // Use final image_data buffer for last thread
            
            // Render the rest of the samples
            cam.samples_per_pixel = remaining_samples;
            threads[num_threads - 1] = std::thread(&render::render_thread, this, std::ref(cam), 
                std::ref(hittables), std::ref(image_data), std::ref(tscanlines[num_threads - 1]));
            sstream << "Created final render thread " << threads[num_threads - 1].get_id() << " for " << cam.samples_per_pixel << " samples." << std::endl;
            addstr(sstream.str().c_str());
            sstream.str("");

            // Polling for thread progress
            bool done = false;
            int startx = 0; int starty = 0;
            getyx(stdscr, starty, startx);
            while(!scanpoll(starty, startx, tscanlines)) {
                ;
            }
            addstr("Render finished!\n");

            // Join threads
            for (std::thread& t : threads) {
                t.join();
                sstream << "Thread joined.\n";
                addstr(sstream.str().c_str());
                sstream.str("");
            }
            refresh();

            // Average the outputs
            for (std::vector<colour>& thread_output : toutput) {
                std::transform(thread_output.begin(), thread_output.end(), image_data.begin(), image_data.begin(), 
                    [](const colour& a, const colour& b){ return a + b; });
            }
            // Output the Image
            outstream << "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

            for (colour c : image_data) {
                write_colour(outstream, c, total_samples);
            }
            outstream << std::flush;
        }

    private:
        int num_threads;

        void render_thread(camera cam, hittable_list& hittables, std::vector<colour>& pixel_data, int& scanlines) {
            cam.render(hittables, pixel_data, scanlines);
        }

        bool scanpoll(int starty, int startx, std::vector<int> tscanlines) {
            int i = 1;
            bool done = true;
            for (int scans : tscanlines) {
                std::string s;
                s.append("Thread ");
                s.append(std::to_string(i));
                s.append(": ");
                s.append(std::to_string(scans));
                s.append("\n");
                mvaddstr(starty + i-1, startx, s.c_str());
                i += 1;
                done = done && (scans == 0);
            }
            refresh();

            return done;
        }
};


#endif // RENDER_H
