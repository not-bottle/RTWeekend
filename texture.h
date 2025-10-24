#ifndef TEXTURE_H
#define TEXTURE_H

#include "vec3.h"
#include "rtw_stb_image.h"

#include <memory>

class texture {
    public:
        virtual ~texture() = default;

        virtual colour value (double u, double v, const point3& p) const = 0;
};

class solid_colour : public texture {
    public:
        solid_colour(const colour& albedo) : albedo(albedo) {}
        
        solid_colour(double red, double green, double blue) : solid_colour(colour(red, green, blue)) {}

        colour value(double u, double v, const point3& p) const override {
            return albedo;
        }

    private:
        colour albedo;
};

class checker_texture : public texture {
    public:
        checker_texture(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
          : inv_scale(1.0 / scale), even(even), odd(odd) {}

        checker_texture(double scale, const colour& c1, const colour& c2)
          : checker_texture(scale, std::make_shared<solid_colour>(c1), std::make_shared<solid_colour>(c2)) {}
        
        colour value(double u, double v, const point3& p) const override {
            auto xInteger = int(std::floor(inv_scale * p.x()));
            auto yInteger = int(std::floor(inv_scale * p.y()));
            auto zInteger = int(std::floor(inv_scale * p.z()));

            bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

            return isEven ? even->value(u, v, p) : odd->value(u, v, p);
        }
        
        
    private:
        double inv_scale;
        std::shared_ptr<texture> even;
        std::shared_ptr<texture> odd;
};

class sky_gradient : public texture {
    public:
        sky_gradient() : colour0(colour(1.0, 1.0, 1.0)), colour1(colour(0.5, 0.7, 1.0)) {}
        sky_gradient(colour colour0, colour colour1) : colour0(colour0), colour1(colour1) {}

        colour value(double u, double v, const point3& p) const override {
            // Treat p as a direction vector here
            vec3 unit_direction = unit_vector(p);
            auto a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*colour0 + a*colour1;
        }
        
    private:
        colour colour0;
        colour colour1;
};

class image_texture : public texture {
    public:
        image_texture(const char* filename) : image(std::make_shared<rtw_image>(filename)) {}
        image_texture(std::shared_ptr<rtw_image> image) : image(image) {}

        colour value(double u, double v, const point3& p) const override {
            // If we have no texture data, then return solid cyan as a debugging aid.
            if (image->height() <= 0) return colour(0, 1, 1);

            // Clamp input texture coordinates to [0,1] x [1,0]
            u = interval(0,1).clamp(u);
            v = 1.0 - interval(0,1).clamp(v); // Flip V to image coordinates

            auto i = int(u * image->width());
            auto j = int(v * image->height());
            auto pixel = image->pixel_data(i,j);

            auto colour_scale = 1.0 / 255.0;
            return colour(colour_scale*pixel[0], colour_scale*pixel[1], colour_scale*pixel[2]);
        }
    
    private:
        std::shared_ptr<rtw_image> image;
};

#endif // TEXTURE_H
