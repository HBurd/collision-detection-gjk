#include "convex_hull.hpp"
#include <limits>

ConvexHullInstance::ConvexHullInstance(demo::math::Vec3 pos, demo::math::Mat3 orient, int mesh_id_)
    : position(pos), orientation(orient), mesh_id(mesh_id_)
{}

demo::math::Vec3 general_support(demo::math::Vec3 dir, const ConvexHullInstance& data, const std::vector<demo::math::Vec3>& vertices)
{
    float max_dot = -std::numeric_limits<float>::infinity();
    demo::math::Vec3 max_dot_v = demo::math::Vec3(0.0f, 0.0f, 0.0f);

    for (const demo::math::Vec3& vertex : vertices)
    {
        demo::math::Vec3 v = data.position + data.orientation * vertex;

        if (dot(dir, v) > max_dot)
        {
            max_dot = dot(dir, v);
            max_dot_v = v;
        }
    }

    return max_dot_v;
}
