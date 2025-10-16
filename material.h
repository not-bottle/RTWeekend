#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"

class hit_record;

class material {
    public:
        virtual ~material() = default; // Virtual Destructor

        /* Virtual function inherited materials need to define.
           Has 3 functions:
              1. Given an incident ray (ray_in), return if the ray was absorbed, or produce a scattered ray.
              2. If the scattered ray is produced, how is the colour attenuated.
        */
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
    public:
        lambertian(const colour& _albedo) : albedo(_albedo) {}

        bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered)
        const override {
            auto scatter_direction = rec.normal + random_unit_vector();

            // If the randomly generated ray is almost (or exactly) opposite the normal, avoid
            // generating a zero ray (as this will never terminate)
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;

            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

    private:
        colour albedo;
};

class metal : public material {
    public:
        metal(const colour& _albedo, double f) : albedo(_albedo), fuzz(f < 1 ? f : 1) {}

        bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered)
        const override {
            auto reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_unit_vector());
            attenuation = albedo;

            // Return false (ray is absorbed) if fuzzed ray is pointing back into the object
            return (dot(scattered.direction(), rec.normal) > 0);
        }
    
    private:
        colour albedo;
        double fuzz;  
};

class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered)
        const override {
            attenuation = colour(1.0, 1.0, 1.0);
            // Refraction ratio is 1/ir if ray is going from air into material,
            // ir/1 if ray is travelling from material into air.
            double refraction_ratio = rec.front_face ? (1.0/ir) : ir;
            auto unit_direction = unit_vector(r_in.direction());

            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

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

            scattered = ray(rec.p, direction);

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

#endif