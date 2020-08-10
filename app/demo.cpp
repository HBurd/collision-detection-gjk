#include "gjk.hpp"
#include "math.hpp"
#include "rendering.hpp"
#include "load_mesh.hpp"

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

            if (filename == "")
            {
                std::cerr << "The load command must be supplied with a file name.\n";
                continue;
            }

            // Check if the filename is a directory
            fs::path path(filename);
            if (fs::is_directory(path))
            {
                std::cout << "Loading meshes in directory " << path << ":\n";

                for (const auto& p : fs::directory_iterator(path))
                {
                    demo::mesh::load_off(p.path().c_str(), io_data.vertices_to_load, io_data.triangles_to_load, io_data.normals_to_load);
                    if (!io_data.vertices_to_load.size())
                    {
                        std::cerr << "Unable to load mesh " << p.path() << ".\n";
                    }
                    else
                    {
                        io_data.mesh_filename = p.path();

                        std::unique_lock lock(io_data.mutex);
                        io_data.load_mesh = true;
                        do
                        {
                            io_data.cv.wait(lock);
                        } while (io_data.load_mesh);
                    }
                }
            }
            else
            {
                demo::mesh::load_off(filename.c_str(), io_data.vertices_to_load, io_data.triangles_to_load, io_data.normals_to_load);
                if (!io_data.vertices_to_load.size())
                {
                    std::cerr << "Unable to load mesh: " << filename << ".\n";
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
                }
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
            else
            {
                std::cerr << "Unknown list option\n";
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
        else if (word == "")
        {
            // Do nothing
        }
        else
        {
            std::cerr << "Unknown command.\n";
        }
    }
}

class KbInputHandler
{
public:
    KbInputHandler(GLFWwindow* window_)
        : window(window_)
    {}

    void register_action(int glfw_key_code, bool is_down_event, std::function<void(GLFWwindow*)>&& action)
    {
        actions.emplace_back(is_down_event, glfw_key_code, std::move(action));
    }

    void do_actions()
    {
        for (auto& action : actions)
        {
            if (glfwGetKey(window, action.glfw_key_code) == GLFW_PRESS)
            {
                if (!(action.is_down_event && action.is_held))
                {
                    action.action(window);
                }

                action.is_held = true;
            }
            else
            {
                action.is_held = false;
            }
        }
    }

private:
    struct RegisteredKey
    {
        bool is_down_event;
        bool is_held = false;

        int glfw_key_code;
        std::function<void(GLFWwindow*)> action;

        RegisteredKey(bool is_down_event_, int key_code, std::function<void(GLFWwindow*)>&& action_)
            : is_down_event(is_down_event_), glfw_key_code(key_code), action(std::move(action_))
        {}
    };

    GLFWwindow* window;
    std::vector<RegisteredKey> actions;
};

int main()
{
    std::vector<Mesh> meshes;
    int selected_mesh = 0;

    IOData io_data;

    std::thread input_thread(input_thread_func, std::ref(io_data));

    RenderContext render_ctxt(800, 600, "SENG 475 Project Demo");

    // TODO: The abstraction shouldn't deal with GLFW
    GLFWwindow* window = render_ctxt.get_glfw_window();

    std::vector<ConvexHullInstance> objects;
    int selected_object = 0;

    // Measure time from the start of the frame
    glfwSetTime(0.0);

    double min_frame_time = 1.0 / 60.0;
    double last_frame_time = 0.0f;

    KbInputHandler kb(window);

    bool update_mesh = false;

    kb.register_action(GLFW_KEY_LEFT, true, [&selected_object, &selected_mesh, &update_mesh] (GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            --selected_mesh;
            update_mesh = true;
        }
        else
        {
            --selected_object;
        }
    });

    kb.register_action(GLFW_KEY_RIGHT, true, [&selected_object, &selected_mesh, &update_mesh] (GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            ++selected_mesh;
            update_mesh = true;
        }
        else
        {
            ++selected_object;
        }
    });

    kb.register_action(GLFW_KEY_UP, true, [&objects, &selected_object, &meshes, selected_mesh] (GLFWwindow*) {
        // Only add an object if a mesh has been loaded
        if (meshes.size() != 0)
        {
            selected_object = objects.size();

            const auto& vertices = meshes[selected_mesh].vertices;

            objects.emplace_back(Vec3::Z(-2.0f), Mat3::Identity(), vertices.data(), vertices.size(), meshes[selected_mesh].render_id);
        }
    });

    kb.register_action(GLFW_KEY_DOWN, true, [&objects] (GLFWwindow*) {
        if (objects.size())
        {
            objects.pop_back();
        }
    });

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

            std::cout << "Loaded mesh " << meshes.back().filename << ".\n";

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
            kb.do_actions();

            // Wrap selected_object and selected_mesh
            if (objects.size() == 0)
            {
                selected_object = 0;
            }
            else if (selected_object >= static_cast<int>(objects.size()) || selected_object < 0)
            {
                while (selected_object < 0)
                {
                    selected_object += objects.size();
                }
                selected_object %= objects.size();
            }

            if (meshes.size() == 0)
            {
                selected_mesh = 0;
            }
            else if (selected_mesh >= static_cast<int>(meshes.size()) || selected_mesh < 0)
            {
                while (selected_mesh < 0)
                {
                    selected_mesh += meshes.size();
                }
                selected_mesh %= meshes.size();
            }

            if (objects.size() != 0)
            {
                auto& object = objects[selected_object];

                if (update_mesh)
                {
                    object.vertices = meshes[selected_mesh].vertices.data();
                    object.vertex_count = meshes[selected_mesh].vertices.size();
                    object.render_id = meshes[selected_mesh].render_id;
                    update_mesh = false;
                }

                float speed = 1.0f; // metres per second
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    object.position += Vec3::Z(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    object.position += Vec3::X(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    object.position += Vec3::Z(last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    object.position += Vec3::X(last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                {
                    object.position += Vec3::Y(-last_frame_time * speed);
                }
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                {
                    object.position += Vec3::Y(last_frame_time * speed);
                }

                float angular_speed = 1.0f; // radians per second
                if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateX(-angular_speed * last_frame_time) * object.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateX(angular_speed * last_frame_time) * object.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateY(-angular_speed * last_frame_time) * object.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateY(angular_speed * last_frame_time) * object.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateZ(angular_speed * last_frame_time) * object.orientation;
                }
                if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
                {
                    object.orientation = Mat3::RotateZ(-angular_speed * last_frame_time) * object.orientation;
                }
            }
        }

        if (objects.size())
        {
            objects[selected_object].colliding = false;
        }

        // Check for collisions with the selected object
        for (int i = 0; i < static_cast<int>(objects.size()); ++i)
        {
            if (i != selected_object)
            {
                auto& object = objects[i];
                geometry::GjkStats stats;
                if (geometry::intersect_gjk(general_support, object, general_support, objects[selected_object], 100, &stats))
                {
                    object.colliding = true;
                    objects[selected_object].colliding = true;
                }
                else
                {
                    object.colliding = false;
                }

                if (stats.iteration_count == 100)
                {
                    std::cout << "GJK did not terminate after 100 iterations" << std::endl;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < static_cast<int>(objects.size()); ++i)
        {
            const auto& object = objects[i];
            render_ctxt.draw_object(object.render_id, object.position, object.orientation.m[0], i == selected_object, object.colliding);
        }

        glfwSwapBuffers(window);

        // Ideally glfwSwapBuffers will synch the frame rate to the vertical
        // retrace rate. This it not guaranteed. If it doesn't, the framerate should still be capped.

        float frame_time = glfwGetTime();

        // TODO: This should have some kind of tolerance
        if (frame_time < min_frame_time)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(min_frame_time - frame_time));
        }
        last_frame_time = glfwGetTime();
        glfwSetTime(0.0);

        glfwPollEvents();
    }

    return 0;
}
