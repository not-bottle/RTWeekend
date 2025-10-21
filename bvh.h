#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
    public: 
        bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {}

        bvh_node(std::vector<std::shared_ptr<hittable>>& objects, size_t start, size_t end) {
            bbox = aabb::empty;
            for (size_t object_index=start; object_index < end; object_index++)
                bbox = aabb(bbox, objects[object_index]->bounding_box());
            
            int axis = bbox.longest_axis();

            auto comparator = (axis == 0) ? box_x_compare
                            : (axis == 1) ? box_y_compare
                                          : box_z_compare;

            size_t object_span = end - start;

            if (object_span == 1) {
                left = right = objects[start];
            } else if (object_span == 2) {
                left = objects[start];
                right = objects[start+1];
            } else {
                std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

                auto mid = start + object_span/2;
                left = std::make_shared<bvh_node>(objects, start, mid);
                right = std::make_shared<bvh_node>(objects, mid, end);
            }
        }

        bool hit(const ray&r, interval ray_bounds, hit_record& rec) const override {
            if (!bbox.hit(r, ray_bounds))
                return false;
            
            // Some recursive properties that make sure we only hit the closest object:
            // - rec only gets filled out when we call the hit method of a leaf node (an actual object)
            // - We explore the left branch completely first
            // - The use of rec.t as an upper bound of the interval sent to right->hit means that any bbox hit
            //   further away than the previous hit will be discarded (at the !bbox.hit above)
            // - ray_bounds.min stays the same as the world bound
            bool hit_left = left->hit(r, ray_bounds, rec);
            bool hit_right = right->hit(r, interval(ray_bounds.min, hit_left ? rec.t : ray_bounds.max), rec);

            return hit_left || hit_right;
        }

        aabb bounding_box() const override { return bbox; }

        private:
            std::shared_ptr<hittable> left;
            std::shared_ptr<hittable> right;
            aabb bbox;

            /* Comparison functions */
            // Generic comparitor that returns the interval with the smallest minimum for a certain index
            static bool box_compare(
                const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b, int axis_index
            ) {
                auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
                auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
                return a_axis_interval.min < b_axis_interval.min;
            }

            static bool box_x_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
                return box_compare(a, b, 0);
            }

            static bool box_y_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
                return box_compare(a, b, 1);
            }

            static bool box_z_compare (const std::shared_ptr<hittable> a, const std::shared_ptr<hittable> b) {
                return box_compare(a, b, 2);
            }
};

#endif