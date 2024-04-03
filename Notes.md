The ray tracer outputs directly to a PPM image file.

## Surface normal direction

In this implementation the surface normals will always face the ray (be pointing in the opposite direction).
This is regardless of whether the ray is facing the "interior" or "exterior" of a surface.
To determine if the ray is facing the "front" or "back" of a surface the front_face value in the hit_record class (defined in hittable.h) is used.
This is calculated at the time of geometry as the direction of the exterior surface normal relative to the ray is used to determine if the ray is on the inside or outside. The surface normal is then flipped if the ray is on the inside. So it is necessary to make this calculation early.

# Diffuse material
- If you set the sky to be a solid colour and the diffuse to preserve 100% of the ray colour you of course end up with a solid colour image.

## Lambertian reflection
Lambertian reflection is a distribution that is more likely to reflect rays in the direction of the surface normal.
This apparently is closer to how "real" diffuse materials work.
The change in the code is a bit esoteric without the background info:

```
vec3 direction = rec.normal + random_unit_vector();
```

All we do is add a random unit vector to the _unit_ normal (important that it is already a unit vector).
This is hard to understand without a diagram, but if you draw a unit sphere around the normal vectors end-point, and then the possible resultant vectors that point from the _surface_ of the object to the edges of this sphere, it's easy to see that vectors pointing in the direction of the normal are more likely.

## Gamma space
The tutorial had a section on "gamma space" which is a bit confusing. The idea is that the colours in the image are presented poorly in most image viewers as they assume the image has been gamma corrected. This means that we need to transform out "linear space" colours in to "gamma space". To take colours from gamma space to linear space we exponentiate by some constant value. This tutorial assumes "gamma space 2", which would mean to square all the colour values. As we need to do this opposite, we take the inverse square (the square root) of all the colour values before outputting them to the file.

## How rays accumulate colours
It is a bit weird how this works. Since all colour values are between 0 and 1 they can just be multiplied.
So if a ray hits a sphere with albedo (0.8, 0.8, 0.2), and then hits the sky with colour (0.8, 0.5, 0.5), these
values just get multiplied together and I guess it works? 
Should explore this more.
