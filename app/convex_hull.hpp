#ifndef CONVEX_HULL_HPP
#define CONVEX_HULL_HPP

#include "math.hpp"

struct ConvexHullInstance
{
    demo::math::Vec3 position;
    demo::math::Mat3 orientation;

    bool colliding = false;

    // A reference to a Mesh is not being used here because the meshes
    // exist in a vector so do not have stable references. The actual
    // vertex data does have a stable reference since it is never modified.
    // This also has the benefit of avoiding a double inderection (i.e.
    // the vertices don't have to be accessed through a Mesh reference).
    const demo::math::Vec3* vertices;
    std::size_t vertex_count;
    std::size_t render_id;

    ConvexHullInstance(demo::math::Vec3 pos, demo::math::Mat3 orient, const demo::math::Vec3* vertices_, std::size_t vertex_count_, std::size_t render_id_);
};

demo::math::Vec3 general_support(demo::math::Vec3 dir, const ConvexHullInstance& data);

#endif
