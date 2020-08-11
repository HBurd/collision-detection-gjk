#include "convex_hull.hpp"

ConvexHullInstance::ConvexHullInstance(demo::math::Vec3 pos, demo::math::Mat3 orient, const demo::math::Vec3* vertices_, std::size_t vertex_count_, std::size_t render_id_)
    : position(pos), orientation(orient), vertices(vertices_), vertex_count(vertex_count_), render_id(render_id_)
{}

demo::math::Vec3 general_support(demo::math::Vec3 dir, const ConvexHullInstance& data)
{
    float max_dot = -1000.0f;   // TODO
    demo::math::Vec3 max_dot_v = demo::math::Vec3(0.0f, 0.0f, 0.0f);

    for (std::size_t i = 0; i < data.vertex_count; ++i)
    {
        demo::math::Vec3 v = data.position + data.orientation * data.vertices[i];
        if (dot(dir, v) > max_dot)
        {
            max_dot = dot(dir, v);
            max_dot_v = v;
        }
    }

    return max_dot_v;
}
