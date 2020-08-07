#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"
#include "load_mesh.hpp"

#include <array>
#include <thread>    // sleep_for needed to enforce framerate
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace demo::math;
using namespace demo::rendering;

struct ConvexHullInstance
{
    Vec3 position;
    Mat3 orientation;

    bool colliding = false;
    std::size_t render_id;

    ConvexHullInstance(Vec3 pos, Mat3 orient, std::size_t id, const Vec3* v, std::size_t v_count)
        : position(pos), orientation(orient), render_id(id), vertices(v), vertex_count(v_count)
    {}

    // These are the points defining the convex hull
    const Vec3* vertices;
    std::size_t vertex_count;
};

// This is for a unit cube
Vec3 general_support(Vec3 dir, const ConvexHullInstance& data)
{
    float max_dot = -1000.0f;   // TODO
    Vec3 max_dot_v = Vec3(0.0f, 0.0f, 0.0f);

    for (std::size_t i = 0; i < data.vertex_count; ++i)
    {
        Vec3 v = data.position + data.orientation * data.vertices[i];
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

    std::vector<Vec3> cube_vertices;
    std::size_t cube_render_id;
    {
        std::vector<Vec3> triangles;
        std::vector<Vec3> normals;

        demo::mesh::load_mesh("demo_meshes/monkey_cvx.off", cube_vertices, triangles, normals);
        if (!triangles.size())
        {
            std::cout << "Unable to load mesh" << std::endl;
        }
        cube_render_id = render_ctxt.load_object(triangles.data(), normals.data(), triangles.size());
    }

    std::vector<ConvexHullInstance> cubes;
    int selected_cube = 0;

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
                    cubes.emplace_back(Vec3::Z(-2.0f), Mat3::Identity(), cube_render_id, cube_vertices.data(), cube_vertices.size());
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
                if (geometry::intersect_gjk(general_support, cube, general_support, cubes[selected_cube], 100, &stats))
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
