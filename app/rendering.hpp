#ifndef RENDERING_HPP
#define RENDERING_HPP

#include "math.hpp"

#include <vector>
#include <cstddef>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace demo::rendering {

    class RenderContext
    {
    public:
        RenderContext(unsigned int w, unsigned int h, const char* title);

        ~RenderContext();

        std::size_t load_object(const demo::math::Vec3* positions, const demo::math::Vec3* normals, std::size_t count);

        void draw_object(std::size_t object_id, const demo::math::Vec3& position, const float* orientation, bool selected, bool colliding, const demo::math::Vec3& global_position, const float* global_orientation);

        GLFWwindow* get_glfw_window();

    private:
        struct ShaderProgram
        {
            GLuint vertex_shader;
            GLuint fragment_shader;
            GLuint program;

            ShaderProgram() = default;

            ShaderProgram(const char* vshader_string, const char* fshader_string);
        };

        struct RenderObject
        {
            GLuint pos_vbo;
            GLuint normal_vbo;
            GLuint vao;
            GLsizei num_vertices;
        };

        GLFWwindow* window;

        unsigned int width;
        unsigned int height;

        float perspective_matrix[16];

        ShaderProgram shader_program;
        std::vector<RenderObject> objects;
    };

    // Returns shader object handle
    GLuint compile_shader(const char* source, GLenum shader_type);

    // Returns program object handle
    GLuint link_shader_program(GLuint vshader, GLuint fshader);

    void make_perspective_matrix(float* data, float near, float far, float fov, float aspect_ratio);
}

#endif
