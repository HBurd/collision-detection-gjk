#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"

#include <array>
#include <thread>    // sleep_for needed to enforce framerate
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace demo::math;
using namespace demo::rendering;

struct CubeData
{
    Vec3 position;
    Mat3 orientation;

    bool colliding = false;
    std::size_t render_id;

    CubeData(Vec3 pos, Mat3 orient, std::size_t id)
        : position(pos), orientation(orient), render_id(id)
    {}

    // These are just points on a convex hull
    static const std::array<Vec3, 8> vertices;
};

const std::array<Vec3, 8> CubeData::vertices = {
    Vec3(0.5f, 0.5f, 0.5f),
    Vec3(0.5f, 0.5f, -0.5f),
    Vec3(0.5f, -0.5f, 0.5f),
    Vec3(0.5f, -0.5f, -0.5f),
    Vec3(-0.5f, 0.5f, 0.5f),
    Vec3(-0.5f, 0.5f, -0.5f),
    Vec3(-0.5f, -0.5f, 0.5f),
    Vec3(-0.5f, -0.5f, -0.5f),
};

// This is for a unit cube
Vec3 cube_support(Vec3 dir, const CubeData& data)
{
    // These are just points on a convex hull
    std::array<Vec3, 8> vertices = {
        data.position + data.orientation * CubeData::vertices[0],
        data.position + data.orientation * CubeData::vertices[1],
        data.position + data.orientation * CubeData::vertices[2],
        data.position + data.orientation * CubeData::vertices[3],
        data.position + data.orientation * CubeData::vertices[4],
        data.position + data.orientation * CubeData::vertices[5],
        data.position + data.orientation * CubeData::vertices[6],
        data.position + data.orientation * CubeData::vertices[7],
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

    // TODO: The abstraction shouldn't deal with GLFW
    GLFWwindow* window = render_ctxt.get_glfw_window();

    std::vector<CubeData> cubes;
    int selected_cube = 0;

    const auto& cube_vertices = CubeData::vertices;
    std::array<Vec3, 36> cube_mesh = {
        cube_vertices[0], cube_vertices[1], cube_vertices[5],    // top face
        cube_vertices[0], cube_vertices[5], cube_vertices[4],

        cube_vertices[0], cube_vertices[4], cube_vertices[2],    // front face
        cube_vertices[2], cube_vertices[4], cube_vertices[6],

        cube_vertices[2], cube_vertices[6], cube_vertices[7],    // bottom face
        cube_vertices[7], cube_vertices[3], cube_vertices[2],

        cube_vertices[3], cube_vertices[7], cube_vertices[5],    // back face
        cube_vertices[5], cube_vertices[1], cube_vertices[3],

        cube_vertices[0], cube_vertices[2], cube_vertices[1],    // right face
        cube_vertices[2], cube_vertices[3], cube_vertices[1],

        cube_vertices[4], cube_vertices[5], cube_vertices[7],    // bottom face
        cube_vertices[7], cube_vertices[6], cube_vertices[4]
    };

    std::array<Vec3, 36> normals = {
        Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f),

        Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f),
        Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f),

        Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f),
        Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f),
        
        Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, -1.0f),
        Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, -1.0f),

        Vec3(1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
        Vec3(1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),

        Vec3(-1.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f),
        Vec3(-1.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f),
    };

    std::size_t cube_render_id = render_ctxt.load_object(cube_mesh.data(), normals.data(), 36);

    // Measure time from the start of the frame
    glfwSetTime(0.0);

    double frame_time_cap = 1.0 / 60.0;
    double last_frame_time = 0.0f;

    bool up_held = false;
    bool down_held = false;
    bool left_held = false;
    bool right_held = false;

    while (!glfwWindowShouldClose(window))
    {
        // Handle input
        {
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                if (!left_held)
                {
                    left_held = true;
                    --selected_cube;
                }
            }
            else
            {
                left_held = false;
            }

            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                if (!right_held)
                {
                    right_held = true;
                    ++selected_cube;
                }
            }
            else
            {
                right_held = false;
            }

            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                if (!up_held)
                {
                    up_held = true;
                    selected_cube = cubes.size();
                    cubes.emplace_back(Vec3::Z(-2.0f), Mat3::Identity(), cube_render_id);
                }
            }
            else
            {
                up_held = false;
            }

            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                if (!down_held)
                {
                    down_held = true;
                    if (cubes.size())
                    {
                        cubes.pop_back();
                    }
                }
            }
            else
            {
                down_held = false;
            }

            // Wrap the selected cube
            if (cubes.size() == 0)
            {
                selected_cube = 0;
            }
            else if (selected_cube >= static_cast<int>(cubes.size()) || selected_cube < 0)
            {
                while (selected_cube < 0)
                {
                    selected_cube += cubes.size();
                }
                selected_cube %= cubes.size();
            }

            if (cubes.size() != 0)
            {
                auto& cube = cubes[selected_cube];

                float speed = 1.0f; // metres per second
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    cube.position += Vec3::Z(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    cube.position += Vec3::X(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    cube.position += Vec3::Z(last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    cube.position += Vec3::X(last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                {
                    cube.position += Vec3::Y(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                {
                    cube.position += Vec3::Y(last_frame_time * speed);
                }

                float angular_speed = 1.0f; // radians per second
                if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateX(-angular_speed * last_frame_time) * cube.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateX(angular_speed * last_frame_time) * cube.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateY(-angular_speed * last_frame_time) * cube.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateY(angular_speed * last_frame_time) * cube.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateZ(angular_speed * last_frame_time) * cube.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
                {
                    cube.orientation = Mat3::RotateZ(-angular_speed * last_frame_time) * cube.orientation;
                }
            }
        }

        if (cubes.size())
        {
            cubes[selected_cube].colliding = false;
        }

        // Check for collisions with the selected cube
        for (int i = 0; i < static_cast<int>(cubes.size()); ++i)
        {
            if (i != selected_cube)
            {
                auto& cube = cubes[i];
                geometry::GjkStats stats;
                if (geometry::intersect_gjk(cube_support, cube, cube_support, cubes[selected_cube], 100, &stats))
                {
                    cube.colliding = true;
                    cubes[selected_cube].colliding = true;
                }
                else
                {
                    cube.colliding = false;
                }

                if (stats.iteration_count == 100)
                {
                    std::cout << "GJK did not terminate after 100 iterations" << std::endl;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < static_cast<int>(cubes.size()); ++i)
        {
            const auto& cube = cubes[i];
            render_ctxt.draw_object(cube.render_id, cube.position, cube.orientation.m[0], i == selected_cube, cube.colliding);
        }

        glfwSwapBuffers(window);

        // Ideally glfwSwapBuffers will synch the frame rate to the vertical
        // retrace rate. This it not guaranteed. If it doesn't, the framerate should still be capped.

        float frame_time = glfwGetTime();

        // TODO: This should have some kind of tolerance
        if (frame_time < frame_time_cap)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(frame_time_cap - frame_time));
        }
        last_frame_time = glfwGetTime();
        glfwSetTime(0.0);

        glfwPollEvents();
    }

    return 0;
}
