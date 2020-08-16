#ifndef CONVEX_HULL_HPP
#define CONVEX_HULL_HPP

#include "math.hpp"
#include <vector>

struct ConvexHullInstance
{
    demo::math::Vec3 position;
    demo::math::Mat3 orientation;

    bool colliding = false;

    // Index of the mesh associated with this object
    int mesh_id;

    ConvexHullInstance(demo::math::Vec3 pos, demo::math::Mat3 orient, int mesh_id_);
};

demo::math::Vec3 general_support(demo::math::Vec3 dir, const ConvexHullInstance& data, const std::vector<demo::math::Vec3>& vertices);

#endif
