#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"

#include <array>
#include <thread>    // sleep_for needed to enforce framerate

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace demo::math;
using namespace demo::rendering;

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
    RenderContext render_ctxt(800, 600, "SENG 475 Project Demo");
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

    // These are just points on a convex hull
    std::array<Vec3, 8> cube_vertices = {
        Vec3(0.5f, 0.5f, 0.5f),
        Vec3(0.5f, 0.5f, -0.5f),
        Vec3(0.5f, -0.5f, 0.5f),
        Vec3(0.5f, -0.5f, -0.5f),
        Vec3(-0.5f, 0.5f, 0.5f),
        Vec3(-0.5f, 0.5f, -0.5f),
        Vec3(-0.5f, -0.5f, 0.5f),
        Vec3(-0.5f, -0.5f, -0.5f),
    };

    std::array<uint32_t, 36> cube_indices = {
        0, 1, 5,    // top face
        0, 5, 4,

        0, 4, 2,    // front face
        2, 4, 6,

        2, 6, 7,    // bottom face
        7, 3, 2,

        3, 7, 5,    // back face
        5, 2, 3,

        0, 2, 1,    // right face
        2, 3, 1,

        4, 5, 7,    // bottom face
        7, 6, 4
    };

    std::size_t cube_id = render_ctxt.load_object(cube_vertices.data(), cube_vertices.size(), cube_indices.data(), cube_indices.size());

    // Measure time from the start of the frame
    glfwSetTime(0.0);

    // Note: just faster than 60fps, to not interfere with vsync
    double frame_time_cap = 1.0 / 61.0;

    while (!glfwWindowShouldClose(render_ctxt.get_glfw_window()))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        Vec3 position(0.0f, 0.0f, -6.0f);
        float orientation[] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        render_ctxt.draw_object(cube_id, position, orientation);

        glfwSwapBuffers(render_ctxt.get_glfw_window());

        // Ideally glfwSwapBuffers will synch the frame rate to the vertical
        // retrace rate. But if it doesn't, the framerate should still be capped.
        // This will cap it to just over 60 fps.

        float frame_time = glfwGetTime();
        if (frame_time < frame_time_cap)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(frame_time_cap - frame_time));
        }

        glfwSetTime(0.0);

        glfwPollEvents();
    }

    return 0;
}
