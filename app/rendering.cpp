#include "rendering.hpp"
#include "math.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using demo::math::Vec3;

namespace demo::rendering {

RenderContext::RenderContext(unsigned int w, unsigned int h, const char* title)
    : width(w), height(h)
{
    assert(glfwInit() != -1);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    assert(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = true;
    assert(glewInit() == GLEW_OK);

    const char* vshader_string =
        "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "uniform mat4 perspective;\n"
        "uniform mat3 orientation;\n"
        "uniform vec3 position;\n"
        "void main() {\n"
        "    vec3 camera_relative = position + orientation * pos;\n"
        "    gl_Position = perspective * vec4(camera_relative, 1.0f);\n"
        "}\n";
    const char* fshader_string =
        "#version 330 core\n"
        "out vec4 colour;\n"
        "void main() {\n"
        "    colour = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n"
        "}\n";

    shader_program = ShaderProgram(vshader_string, fshader_string);

    // TODO: adjustable near/far/fov
    make_perspective_matrix(perspective_matrix, 0.1f, 100.0f, 1.0f, float(width) / float(height));
}

RenderContext::~RenderContext()
{
    glfwTerminate();
}

std::size_t RenderContext::load_object(const Vec3* positions, std::size_t pos_count,
    const uint32_t* indices, std::size_t index_count)
{
    GLuint pos_vbo;
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * pos_count, positions, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);   // Location for position in shader program
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    GLuint index_buf;
    glGenBuffers(1, &index_buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * index_count, indices, GL_STATIC_DRAW);

    std::size_t object_id = objects.size();
    objects.emplace_back();

    objects.back().pos_vbo = pos_vbo;
    objects.back().index_buf = index_buf;
    objects.back().vao = vao;
    objects.back().num_faces = index_count / 3;

    return object_id;
}

void RenderContext::draw_object(std::size_t object_id, const Vec3& position, const float* orientation)
{
    glUseProgram(shader_program.program);

    const RenderObject& object = objects[object_id];

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.index_buf);
    glBindVertexArray(object.vao);

    GLint location = glGetUniformLocation(shader_program.program, "perspective");
    glUniformMatrix4fv(location, 1, GL_TRUE, perspective_matrix);

    location = glGetUniformLocation(shader_program.program, "orientation");
    glUniformMatrix3fv(location, 1, GL_TRUE, orientation);

    location = glGetUniformLocation(shader_program.program, "position");
    glUniform3f(location, position.x, position.y, position.z);

    glDrawElements(GL_TRIANGLES, object.num_faces, GL_UNSIGNED_INT, 0);
}

GLFWwindow* RenderContext::get_glfw_window()
{
    return window;
}

RenderContext::ShaderProgram::ShaderProgram(const char* vshader_string, const char* fshader_string)
{
    vertex_shader = compile_shader(vshader_string, GL_VERTEX_SHADER);
    fragment_shader = compile_shader(fshader_string, GL_FRAGMENT_SHADER);
    program = link_shader_program(vertex_shader, fragment_shader);
}

// Returns shader object handle
GLuint compile_shader(const char* source, GLenum shader_type)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<GLchar> log(log_size);
        glGetShaderInfoLog(shader, log_size, &log_size, log.data());

        std::cerr << log.data() << std::endl;
    }
    return shader;
}

// Returns program object handle
GLuint link_shader_program(GLuint vshader, GLuint fshader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<GLchar> log(log_size);
        glGetProgramInfoLog(program, log_size, &log_size, log.data());

        std::cerr << log.data() << std::endl;
    }

    return program;
}

void make_perspective_matrix(float* data, float near, float far, float fov, float aspect_ratio)
{
    float csc_fov = 1.0f / sinf(fov * 0.5f);
    float matrix_data[16] =
    {
        csc_fov, 0.0f, 0.0f, 0.0f,
        0.0f, aspect_ratio * csc_fov, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -2 * near * far / (far - near),
        0.0f, 0.0f, -1.0f, 0.0f
    };

    std::copy_n(matrix_data, 16, data);
}

}
