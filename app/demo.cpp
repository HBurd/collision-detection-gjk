#include "gjk.hpp"
#include "math.hpp"

#include <array>

using namespace demo::math;

struct CubeData
{
    Mat3 orientation;
};

Vec3 cube_support(Vec3 dir, const CubeData& data)
{
    // These are just points on a convex hull
    std::array<Vec3, 8> vertices = {
        data.orientation * Vec3(0.5f, 0.5f, 0.5f),
        data.orientation * Vec3(0.5f, 0.5f, -0.5f),
        data.orientation * Vec3(0.5f, -0.5f, 0.5f),
        data.orientation * Vec3(0.5f, -0.5f, -0.5f),
        data.orientation * Vec3(-0.5f, 0.5f, 0.5f),
        data.orientation * Vec3(-0.5f, 0.5f, -0.5f),
        data.orientation * Vec3(-0.5f, -0.5f, 0.5f),
        data.orientation * Vec3(-0.5f, -0.5f, -0.5f),
    };

    for (auto v : vertices)
    {

    }
}

int main()
{

    return 0;
}
