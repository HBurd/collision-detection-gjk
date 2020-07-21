#include "gjk.hpp"
#include "math.hpp"

#include <array>
#include <GLFW/glfw3.h>

using namespace demo::math;

struct CubeData
{
    Vec3 position;
    Mat3 orientation;
};

// This is for a unit cube
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

    // Note: A little sketchy
    float max_dot = -1000.0f;
    Vec3 max_dot_v = Vec3(0.0f, 0.0f, 0.0f);

    for (auto v : vertices)
    {
        if (dot(dir, v) > max_dot)
        {
            max_dot = dot(dir, v);
            max_dot_v = v;
        }
    }

    return max_dot_v;
}

int main()
{

    assert(glfwInit() != -1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SENG 475 Project Demo", nullptr, nullptr);
    assert(window);


    //CubeData data1;
    //CubeData data2;
    //data2.position = Vec3(2.0f, 0.0f, 0.0f);

    //assert(!geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    //data2.position = Vec3(0.5f, 0.0f, 0.0f);
    //assert(geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    //data2.position = Vec3(0.0f, 1.01f, 0.0f);
    //assert(!geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    //data2.position = Vec3(0.0f, 0.99f, 0.0f);
    //assert(geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    //data2.position = Vec3(0.99f, 0.99f, 0.99f);
    //assert(geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    //data2.position = Vec3(1.01f, 0.99f, 0.99f);
    //assert(!geometry::intersect_gjk(cube_support, data1, cube_support, data2));

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
