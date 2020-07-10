#include "gjk.hpp"
#include "math.hpp"

#include <array>

using namespace demo::math;

struct CubeData
{
    Vec3 position;
    Mat3 orientation;
};

Vec3 cube_support(Vec3 dir, const CubeData& data)
{
    // These are just points on a convex hull
    std::array<Vec3, 8> vertices = {
        data.position + data.orientation * Vec3(0.5f, 0.5f, 0.5f),
        data.position + data.orientation * Vec3(0.5f, 0.5f, -0.5f),
        data.position + data.orientation * Vec3(0.5f, -0.5f, 0.5f),
        data.position + data.orientation * Vec3(0.5f, -0.5f, -0.5f),
        data.position + data.orientation * Vec3(-0.5f, 0.5f, 0.5f),
        data.position + data.orientation * Vec3(-0.5f, 0.5f, -0.5f),
        data.position + data.orientation * Vec3(-0.5f, -0.5f, 0.5f),
        data.position + data.orientation * Vec3(-0.5f, -0.5f, -0.5f),
    };

    float max_dot = 0.0f;
    Vec3 max_dot_v = Vec3(0.0f, 0.0f, 0.0f);

    for (auto v : vertices)
    {
        max_dot = std::max(max_dot, dot(dir, v));
        max_dot_v = v;
    }

    return max_dot_v;
}

int main()
{
    CubeData data1;
    CubeData data2;
    data2.position = Vec3(1.0f, 1.0f, 1.0f);

    geometry::intersect_gjk(cube_support, data1, cube_support, data2);

    return 0;
}
