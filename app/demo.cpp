#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"
#include "load_mesh.hpp"

#include <array>
#include <thread>    // sleep_for needed to enforce framerate
#include <iostream>
#include <iomanip>
#include <sstream>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace demo::math;
using namespace demo::rendering;

struct Mesh
{
    std::size_t render_id;
    std::vector<Vec3> vertices;
    std::string filename;

    Mesh(std::size_t render_id_, std::string&& filename_)
        : render_id(render_id_), filename(filename_)
    {}
};

struct ConvexHullInstance
{
    Vec3 position;
    Mat3 orientation;

    bool colliding = false;

    // A reference to a Mesh is not being used here because the meshes
    // exist in a vector so do not have stable references. The actual
    // vertex data does have a stable reference since it is never modified.
    // This also has the benefit of avoiding a double inderection (i.e.
    // the vertices don't have to be accessed through a Mesh reference).
    const Vec3* vertices;
    std::size_t vertex_count;
    std::size_t render_id;

    ConvexHullInstance(Vec3 pos, Mat3 orient, const Vec3* vertices_, std::size_t vertex_count_, std::size_t render_id_)
        : position(pos), orientation(orient), vertices(vertices_), vertex_count(vertex_count_), render_id(render_id_)
    {}
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

struct IOData
{
    std::atomic_bool load_mesh = false;
    std::string mesh_filename;
    std::vector<Vec3> vertices_to_load;
    std::vector<Vec3> triangles_to_load;
    std::vector<Vec3> normals_to_load;

    std::atomic_bool list_mesh = false;

    std::atomic_bool select_mesh = false;
    std::size_t selected_mesh = 0;

    std::mutex mutex;
    std::condition_variable cv;
};

void input_thread_func(IOData& io_data)
{
    bool do_input = true;
    while (do_input)
    {
        std::cout << "> ";

        std::string command;
        std::getline(std::cin, command);

        std::stringstream command_sstream(command);

        std::string word;
        command_sstream >> word;

        if (word == "load")
        {
            // Command to load mesh

            std::string filename;
            command_sstream >> filename;

            demo::mesh::load_mesh(filename.c_str(), io_data.vertices_to_load, io_data.triangles_to_load, io_data.normals_to_load);
            if (!io_data.vertices_to_load.size())
            {
                std::cout << "Unable to load mesh" << std::endl;
            }
            else
            {
                io_data.mesh_filename = std::move(filename);

                std::unique_lock lock(io_data.mutex);
                io_data.load_mesh = true;
                do
                {
                    io_data.cv.wait(lock);
                } while (io_data.load_mesh);
                std::cout << "mesh loaded" << std::endl;
            }
        }
        else if (word == "list")
        {
            command_sstream >> word;

            if (word == "mesh")
            {
                std::unique_lock lock(io_data.mutex);
                io_data.list_mesh = true;
                do
                {
                    io_data.cv.wait(lock);
                } while (io_data.list_mesh);
            }
        }
        else if (word == "mesh")
        {
            command_sstream >> io_data.selected_mesh;

            std::unique_lock lock(io_data.mutex);
            io_data.select_mesh = true;
            do
            {
                io_data.cv.wait(lock);
            } while (io_data.select_mesh);
        }
    }
}

int main()
{
    std::vector<Mesh> meshes;
    int selected_mesh = 0;

    IOData io_data;

    std::thread input_thread(input_thread_func, std::ref(io_data));

    RenderContext render_ctxt(800, 600, "SENG 475 Project Demo");

    // TODO: The abstraction shouldn't deal with GLFW
    GLFWwindow* window = render_ctxt.get_glfw_window();

    {
        std::vector<Vec3> vertices;
        std::vector<Vec3> triangles;
        std::vector<Vec3> normals;

        std::string demo_filename("demo_meshes/monkey_cvx.off");

        demo::mesh::load_mesh(demo_filename.c_str(), vertices, triangles, normals);
        if (!triangles.size())
        {
            std::cout << "Unable to load mesh" << std::endl;
        }
        meshes.emplace_back(render_ctxt.load_object(triangles.data(), normals.data(), triangles.size()), std::move(demo_filename));
        meshes.back().vertices = std::move(vertices);
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
        // Handle input from the input thread
        if (io_data.load_mesh)
        {
            std::scoped_lock lock(io_data.mutex);

            selected_mesh = meshes.size();

            meshes.emplace_back(render_ctxt.load_object(
                    io_data.triangles_to_load.data(),
                    io_data.normals_to_load.data(),
                    io_data.triangles_to_load.size()),
                std::move(io_data.mesh_filename));
            meshes.back().vertices = std::move(io_data.vertices_to_load);

            io_data.load_mesh = false;
            io_data.cv.notify_one();
        }
        if (io_data.list_mesh)
        {
            std::scoped_lock lock(io_data.mutex);

            std::cout << "Mesh ID   Number of Vertices   Filename\n";
            for (std::size_t i = 0; i < meshes.size(); ++i)
            {
                std::cout << std::left << std::setw(10) << i << std::setw(21) << meshes[i].vertices.size() << meshes[i].filename << "\n";
            }

            io_data.list_mesh = false;
            io_data.cv.notify_one();
        }
        if (io_data.select_mesh)
        {
            std::scoped_lock lock(io_data.mutex);

            if (io_data.selected_mesh < meshes.size())
            {
                selected_mesh = io_data.selected_mesh;
                std::cout << "Mesh " << selected_mesh << " selected.\n";
            }
            else
            {
                std::cout << "Error: This mesh does not exist.\n";
            }

            io_data.select_mesh = false;
            io_data.cv.notify_one();
        }

        // Handle keyboard input
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

                    const auto& vertices = meshes[selected_mesh].vertices;

                    cubes.emplace_back(Vec3::Z(-2.0f), Mat3::Identity(), vertices.data(), vertices.size(), meshes[selected_mesh].render_id);
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
