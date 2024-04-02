# RTWeekend

The ray tracer outputs directly to a PPM image file.

## Surface normal direction

In this implementation the surface normals will always face the ray (be pointing in the opposite direction).
This is regardless of whether the ray is facing the "interior" or "exterior" of a surface.
To determine if the ray is facing the "front" or "back" of a surface the front_face value in the hit_record class (defined in hittable.h) is used.
This is calculated at the time of geometry as the direction of the exterior surface normal relative to the ray is used to determine if the ray is on the inside or outside. The surface normal is then flipped if the ray is on the inside. So it is necessary to make this calculation early.

## vec3.h

Defines a general purpose 3D vector class, overloading operators to work as expected.

## colour.h

## ray.h

## hittable.h

## hittable_list.h

## sphere.h
