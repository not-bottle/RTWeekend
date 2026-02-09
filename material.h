#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "texture.h"
#include "pdf.h"

#include <algorithm>

class hit_record;

class scatter_record {
    public:
        colour attenuation;
        std::shared_ptr<pdf> pdf_ptr;
        bool skip_pdf;
        ray skip_pdf_ray;
};

class material {
    public:
        virtual ~material() = default; // Virtual Destructor

        virtual colour emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
            return colour(0,0,0);
        }

        /* Virtual function inherited materials need to define.
           Has 3 functions:
              1. Given an incident ray (ray_in), return if the ray was absorbed, or produce a scattered ray.
              2. If the scattered ray is produced, how is the colour attenuated.
        */
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, scatter_record& srec)
            const { return false; };

        virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) 
            const { return 0; }

        bool noshade = false;
};

class lambertian : public material {
    public:
        lambertian(const colour& albedo) : tex(std::make_shared<solid_colour>(albedo)) {}
        lambertian(std::shared_ptr<texture> tex) : tex(tex) {}

        bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) 
        const override {
            srec.attenuation = tex->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = std::make_shared<cosine_pdf>(rec.normal);
            srec.skip_pdf = false;
            return true;
        }

        double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
        {
            auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
            return cos_theta < 0 ? 0 : cos_theta/pi;
        }

    private:
        std::shared_ptr<texture> tex;
};

class metal : public material {
    public:
        metal(std::shared_ptr<texture> tex, double f) : tex(tex), fuzz(f < 1 ? f : 1) {}
        metal(const colour& albedo, double f) : metal(std::make_shared<solid_colour>(albedo), fuzz) {}

        bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) 
        const override {
            auto reflected = reflect(r_in.direction(), rec.normal);
            reflected = unit_vector(reflected) + (fuzz*random_unit_vector());

            srec.attenuation = tex->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = nullptr;
            srec.skip_pdf = true;
            srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());

            return true;
        }

    private:
        std::shared_ptr<texture> tex;
        double fuzz;
};

class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec)
        const override {
            srec.attenuation = colour(1.0, 1.0, 1.0);
            srec.pdf_ptr = nullptr;
            srec.skip_pdf = true;
            // Refraction ratio is 1/ir if ray is going from air into material,
            // ir/1 if ray is travelling from material into air.
            double refraction_ratio = rec.front_face ? (1.0/ir) : ir;
            auto unit_direction = unit_vector(r_in.direction());

            double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

            // Account for total internal reflection (when the ray cannot refract)
            // Note: This can only occur when ir is positive (ray is in the material
            // with the higher refraction index)
            bool cannot_refract = refraction_ratio*sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
                // Rays that cannot refract are reflected internally
                direction = reflect(unit_direction, rec.normal);
            } else {
                direction = refract(unit_direction, rec.normal, refraction_ratio);
            }

            srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());

            return true;
        }

    private:
        double ir; // Index of refraction

        static double reflectance(double cosine, double ref_idx) {
            // Use Schlik's approximation for reflectance
            auto r0 = (1- ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*pow((1 - cosine), 5);
        }
};

class diffuse_light : public material {
    public:
        diffuse_light(std::shared_ptr<texture> tex) : tex(tex) {}
        diffuse_light(const colour& emit) : tex(std::make_shared<solid_colour>(emit)) {}

        colour emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override {
            if (!rec.front_face)
                return colour(0.0,0.0,0.0);
            return tex->value(u, v, p);
        }

    private:
        std::shared_ptr<texture> tex;
    
};

class isotropic : public material {
    public:
        isotropic(const colour& albedo) : tex(std::make_shared<solid_colour>(albedo)) {}
        isotropic(std::shared_ptr<texture> tex) : tex(tex) {}

        bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec)
        const override {
            srec.attenuation = tex->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = std::make_shared<sphere_pdf>();
            srec.skip_pdf = false;
            return true;
        }

        double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
        const override {
            return 1 / (4 * pi);
        }
    private:
        std::shared_ptr<texture> tex;
};

//class facing_ratio : public material {
//    public:
//        facing_ratio() { noshade = true; }
//    bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered, double& pdf)
//        const override {
//            scattered = ray(rec.p, rec.normal, r_in.time());
//            attenuation = colour(std::max(dot(unit_vector(rec.normal), unit_vector(-r_in.dir)), 0.0));
//            return true;
//        }
//};
//
//class shade_normal : public material {
//    public:
//        shade_normal() { noshade = true; }
//
//    bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered, double& pdf)
//        const override {
//            scattered = ray(rec.p, rec.normal, r_in.time());
//            attenuation = colour(rec.normal);
//            return true;
//        }
//};

#endif
