#ifndef AABB_H
#define AABB_H

#include "vec3.h"
#include "interval.h"

class aabb {
    public:
    interval x, y, z;

    aabb() {};
    
    aabb(interval x, interval y, interval z)
     : x(x), y(y), z(z) 
     {
         pad_to_minimums();
     }

     aabb(const point3& a, const point3& b) {
         x = a[0] <= b[0] ? interval(a[0], b[0]) : interval(b[0], a[0]);
         y = a[1] <= b[1] ? interval(a[1], b[1]) : interval(b[1], a[1]);
         z = a[2] <= b[2] ? interval(a[2], b[2]) : interval(b[2], a[2]);

         pad_to_minimums();
     }

     aabb(const aabb& a, const aabb&b) {
         x = interval(a.x, b.x);
         y = interval(a.y, b.y);
         z = interval(a.z, b.z);
     }

     const interval& axis_interval(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
     }

     int longest_axis() const {
         // Returns index of longest axis of bounding box
         if (x.size() > y.size()) {
            return x.size() > z.size() ? 0 : 2;
         } else {
            return y.size() > z.size() ? 1 : 2;
         }
     }

     bool hit (const ray& r, interval ray_bounds) const
     {
      const point3& ray_orig = r.origin();
      const vec3& ray_dir = r.direction();

      for (int axis = 0; axis < 3; axis++) {
         const interval& ax = axis_interval(axis);
         const double adinv = 1.0 / ray_dir[axis];

         auto t0 = (ax.min - ray_orig[axis]) * adinv;
         auto t1 = (ax.max - ray_orig[axis]) * adinv;

         if (t0 < t1) {
            if (t0 > ray_bounds.min) ray_bounds.min = t0;
            if (t1 < ray_bounds.max) ray_bounds.max = t1;
         } else {
            if (t1 > ray_bounds.min) ray_bounds.min = t1;
            if (t0 < ray_bounds.max) ray_bounds.max = t0;
         }

         if (ray_bounds.max <= ray_bounds.min)
            return false;
      }
      return true;
     }

     static const aabb empty, universe;

     friend std::ostream& operator<< (std::ostream& out, const aabb& bbox);

     private:
         void pad_to_minimums() {
            double delta = 0.0001;
            if (x.size() < delta) x = x.expand(delta);
            if (y.size() < delta) y = y.expand(delta);
            if (z.size() < delta) z = z.expand(delta);
         }
};

const aabb aabb::empty    = aabb(interval::empty,    interval::empty,    interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
   return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

aabb operator+(const vec3& offset, const aabb& bbox) {
   return bbox + offset;
}

std::ostream& operator<< (std::ostream& out, const aabb& bbox) 
{
    out << "x" << bbox.x << std::endl;
    out << "y" << bbox.y << std::endl;
    out << "z" << bbox.z << std::endl;

    return out;
}

#endif