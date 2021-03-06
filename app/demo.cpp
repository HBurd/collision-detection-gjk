#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"
#include "load_mesh.hpp"
#include "input.hpp"
#include "convex_hull.hpp"

#include <array>
#include <thread>    // sleep_for needed to enforce framerate
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace fs = std::filesystem;

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

struct InputCommands
{
    void handle_commands(RenderContext& render_ctxt, std::vector<Mesh>& meshes, int& currently_selected_mesh)
    {
        // Handle input from the input thread
        if (load_mesh)
        {
            std::vector<Vec3> vertices;
            std::vector<Vec3> triangles;
            std::vector<Vec3> normals;

            // Check if the filename is a directory
            fs::path path(mesh_filename);
            if (fs::is_directory(path))
            {
                std::cout << "Loading meshes in directory " << path << ":\n";

                for (const auto& p : fs::directory_iterator(path))
                {
                    demo::mesh::load_off(p.path().c_str(), vertices, triangles, normals);
                    if (vertices.size() && triangles.size() && normals.size())
                    {
                        meshes.emplace_back(
                            render_ctxt.load_object(
                                triangles.data(),
                                normals.data(),
                                triangles.size()),
                            p.path());
                        meshes.back().vertices = std::move(vertices);
                        std::cout << "Loaded mesh " << p.path() << ".\n";
                    }
                    else
                    {
                        std::cerr << "Unable to load mesh " << p.path() << ".\n";
                    }
                }
            }
            else
            {
                demo::mesh::load_off(path.c_str(), vertices, triangles, normals);
                if (vertices.size() && triangles.size() && normals.size())
                {
                    meshes.emplace_back(
                        render_ctxt.load_object(
                            triangles.data(),
                            normals.data(),
                            triangles.size()),
                        path);
                    meshes.back().vertices = std::move(vertices);
                    std::cout << "Loaded mesh " << path << ".\n";
                }
                else
                {
                    std::cerr << "Unable to load mesh: " << path << ".\n";
                }
            }

            std::scoped_lock lock(mutex);
            load_mesh = false;
            cv.notify_one();
        }
        if (list_mesh)
        {
            std::scoped_lock lock(mutex);

            std::cout << "Mesh ID   Number of Vertices   Filename\n";
            for (std::size_t i = 0; i < meshes.size(); ++i)
            {
                std::cout << std::left << std::setw(10) << i << std::setw(21) << meshes[i].vertices.size() << meshes[i].filename << "\n";
            }

            list_mesh = false;
            cv.notify_one();
        }
        if (select_mesh)
        {
            std::scoped_lock lock(mutex);

            if (selected_mesh < meshes.size())
            {
                currently_selected_mesh = selected_mesh;
                std::cout << "Mesh " << selected_mesh << " selected.\n";
            }
            else
            {
                std::cout << "Error: This mesh does not exist.\n";
            }

            select_mesh = false;
            cv.notify_one();
        }
    }

    std::atomic_bool load_mesh = false;
    std::string mesh_filename;

    std::atomic_bool list_mesh = false;

    std::atomic_bool select_mesh = false;
    std::size_t selected_mesh = 0;

    std::atomic_bool quit = false;

    std::mutex mutex;
    std::condition_variable cv;
};

void input_thread_func(InputCommands& io_data)
{
    // Wait for the previous command to finish
    {
        std::unique_lock lock(io_data.mutex);
        while ((io_data.load_mesh || io_data.list_mesh || io_data.select_mesh) && !io_data.quit)
        {
            io_data.cv.wait(lock);
        }
    }

    while (!io_data.quit)
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

            if (filename == "")
            {
                std::cerr << "The load command must be supplied with a file name.\n";
            }
            else
            {
                io_data.mesh_filename = std::move(filename);
            }

            io_data.load_mesh = true;
        }
        else if (word == "list")
        {
            command_sstream >> word;

            if (word == "mesh")
            {
                io_data.list_mesh = true;
            }
            else
            {
                std::cerr << "Unknown list option\n";
            }
        }
        else if (word == "mesh")
        {
            command_sstream >> io_data.selected_mesh;
            io_data.select_mesh = true;
        }
        else if (word == "exit" || word == "quit")
        {
            io_data.quit = true;
        }
        else if (word == "")
        {
            // Do nothing
        }
        else
        {
            std::cerr << "Unknown command.\n";
        }

        // Wait for previous command to finish
        {
            std::unique_lock lock(io_data.mutex);
            while ((io_data.load_mesh || io_data.list_mesh || io_data.select_mesh) && !io_data.quit)
            {
                io_data.cv.wait(lock);
            }
        }
    }
}

// Returns the unique representative of the a mod b equivalence class in the range [0, b),
// or returns zero if b is zero.
int modulo(int a, unsigned int b)
{
    // % Returns remainder, so it has to be made positive for when a is negative.
    if (b)
    {
        return (a % static_cast<int>(b) + b) % static_cast<int>(b);
    }
    
    return 0;
}

int main(int argc, char** args)
{
    std::vector<Mesh> meshes;
    int selected_mesh = 0;

    InputCommands io_data;

    // Handle command line arguments
    if (argc == 2)
    {
        // The user supplied a file or directory to load
        io_data.mesh_filename = args[1];
        io_data.load_mesh = true;
    }
    else if (argc > 2)
    {
        // This is not valid usage
        std::cerr << "usage: demo [directory_to_load]\n";
    }

    std::thread input_thread(input_thread_func, std::ref(io_data));

    RenderContext render_ctxt(800, 600, "SENG 475 Project Demo");

    GLFWwindow* window = render_ctxt.get_glfw_window();

    std::vector<ConvexHullInstance> objects;
    int selected_object = 0;

    Vec3 global_position(0.0f, 0.0f, -10.0f);
    Mat3 global_orientation;

    // This handles key events. In particular, it catches the event when the key is pressed down,
    // rather than just detecting if the key is pressed.
    InputHandler input(window);

    // Register the events for the arrow keys (which only trigger as they are pressed).
    input.register_action(GLFW_KEY_LEFT, true, [&selected_object, &objects, &meshes, &selected_mesh, &input] {
        if (input.get_shift())
        {
            selected_mesh = modulo(selected_mesh - 1, meshes.size());
        }
        else
        {
            if (objects.size())
            {
                selected_object = modulo(selected_object - 1, objects.size());
                selected_mesh = objects[selected_object].mesh_id;
            }
        }
    });

    input.register_action(GLFW_KEY_RIGHT, true, [&selected_object, &objects, &meshes, &selected_mesh, &input] {
        if (input.get_shift())
        {
            selected_mesh = modulo(selected_mesh + 1, meshes.size());
        }
        else
        {
            if (objects.size())
            {
                selected_object = modulo(selected_object + 1, objects.size());
                selected_mesh = objects[selected_object].mesh_id;
            }
        }
    });

    input.register_action(GLFW_KEY_UP, true, [&objects, &selected_object, &meshes, &selected_mesh] {
        // Only add an object if a mesh has been loaded
        if (meshes.size() != 0)
        {
            // Select the new object when it is created.
            selected_object = objects.size();

            objects.emplace_back(Vec3(), Mat3::Identity(), selected_mesh);
        }
    });

    input.register_action(GLFW_KEY_DOWN, true, [&objects, &selected_object, &selected_mesh] {
        if (objects.size())
        {
            // Remove the selected object, and replace it with the object at the back.
            objects[selected_object] = objects.back();
            objects.pop_back();

            // selected_mesh might now be invalidated, if the object at the back was removed.
            // So it needs to be set to a valid index.
            selected_object = 0;
            selected_mesh = objects[selected_object].mesh_id;
        }
    });

    double min_frame_time = 1.0 / 60.0;
    double last_frame_time = 0.0f;

    // Measure time from the start of the frame
    glfwSetTime(0.0);

    while (!input.window_should_close() && !io_data.quit)
    {
        io_data.handle_commands(render_ctxt, meshes, selected_mesh);

        input.do_actions();

        if (objects.size() != 0)
        {
            auto& object = objects[selected_object];

            object.mesh_id = selected_mesh;

            bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

            float speed = 1.0f; // metres per second
            Vec3 velocity_vector = speed * input.get_wasdqe_vector();

            // Rotate the applied velocity into the camera reference frame
            velocity_vector = global_orientation.transpose() * velocity_vector;

            object.position += last_frame_time * velocity_vector;

            float angular_speed = 1.0f; // radians per second
            Vec3 angular_velocity = angular_speed * input.get_ijkluo_vector();
            angular_velocity = Vec3(angular_velocity.z, angular_velocity.x, -angular_velocity.y);

            if (shift_pressed)
            {
                global_orientation = Mat3::AxisAngle(last_frame_time * angular_velocity) * global_orientation;
            }
            else
            {
                // Rotate the applied angular velocity into the camera reference frame
                angular_velocity = global_orientation.transpose() * angular_velocity;
                object.orientation = Mat3::AxisAngle(last_frame_time * angular_velocity) * object.orientation;
            }
        }

        // Forget previous intersections
        for (auto& object : objects)
        {
            object.colliding = false;
        }

        // Check for intersections between every pair of objects.
        for (int i = 0; i < static_cast<int>(objects.size() - 1); ++i)
        {
            // Start at 1 past i so that each intersection is only checked
            // once, and never with itself.
            for (int j = i + 1; j < static_cast<int>(objects.size()); ++j)
            {
                geometry::GjkStats stats;
                bool intersection = geometry::intersect_gjk<Vec3>(
                    [&objects, &meshes, i](const Vec3& d) { return general_support(d, objects[i], meshes[objects[i].mesh_id].vertices); },
                    [&objects, &meshes, j](const Vec3& d) { return general_support(d, objects[j], meshes[objects[j].mesh_id].vertices); },
                    100, &stats);

                objects[i].colliding |= intersection;
                objects[j].colliding |= intersection;

                if (stats.iteration_count == 100)
                {
                    std::cerr << "GJK did not terminate after 100 iterations" << std::endl;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < static_cast<int>(objects.size()); ++i)
        {
            const auto& object = objects[i];
            render_ctxt.draw_object(meshes[object.mesh_id].render_id, object.position, object.orientation.m[0], i == selected_object, object.colliding, global_position, global_orientation.m[0]);
        }

        glfwSwapBuffers(window);

        // Ideally glfwSwapBuffers will synch the frame rate to the vertical
        // retrace rate. This it not guaranteed. If it doesn't, the framerate should still be capped.

        float frame_time = glfwGetTime();

        // TODO: This should have some kind of tolerance, so that
        // being slightly too fast doesn't trigger sleeping (which
        // probably doesn't support fine-enough time resolutions).
        if (frame_time < min_frame_time)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(min_frame_time - frame_time));
        }
        last_frame_time = glfwGetTime();
        glfwSetTime(0.0);

        input.poll_events();
    }

    // The input thread can only be detached here, since it will be blocked on standard input
    // (unless the program was exited from the console).
    input_thread.detach();

    return 0;
}
