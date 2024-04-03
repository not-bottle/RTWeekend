#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"

class hit_record;

class material {
    public:
        virtual ~material() = default;

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

#endif