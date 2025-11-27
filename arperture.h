#if !defined(ARPERTURE_H)
#define ARPERTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"

class arperture {
    public:
        arperture(std::shared_ptr<rtw_image> tex) {
            arp = std::make_shared<std::vector<vec3>>();
            vec3 centre = vec3(((float)tex->width() - 1) / 2.0, ((float)tex->height() - 1) / 2.0, 0.0);
            for (int i = 0; i < tex->height(); i++) {
                for (int j = 0; j < tex->width(); j++)
                {
                    auto p = (uint8_t*)tex->pixel_data(j, tex->height()-i);
                    int r  = *p;
                    int g  = *(p+1);
                    int b  = *(p+2);
                    // std::cerr << "(" << r << ", " << g << ", " << b << ") ";
                    if (r == 255 && b == 255 && g == 255) {
                        vec3 d = vec3(j, i, 0) - centre;
                        d = vec3(d.x() / tex->width() * 2, d.y() / tex->height() * 2, 0);
                        //std::cerr << "(" << d << ") ";
                        arp->push_back(d);
                    }

                }
                //std::cerr << std::endl;
            }
        }

    point3 sample(point3 centre, vec3 u, vec3 v) {
        vec3 p = arp->at(random_int(0, arp->size() - 1));
        return centre + (p[0] * u) + (p[1] * v);
    }
    
    private:
        // Contains vectors pointing to valid arpeture samples from the camera centre (normalized to within unit square)
        std::shared_ptr<std::vector<vec3>> arp;
};

#endif // ARPERTURE_H