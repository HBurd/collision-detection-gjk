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

    // Initialize GLEW
    glewExperimental = true;
    assert(glewInit() == GLEW_OK);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Only show the front of faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Vertex shader description:
    // Computes camera-relative normals and perspective-transformed vertex coordinates
    const char* vshader_string =
        "#version 140\n"
        "in vec3 vpos;\n"
        "in vec3 normal;\n"
        "uniform mat4 perspective;\n"
        "uniform mat3 orientation;\n"
        "uniform vec3 position;\n"
        "uniform vec3 global_position;\n"
        "uniform mat3 global_orientation;\n"
        "out vec3 camera_relative_normal;\n"
        "out vec3 camera_relative_position;\n"
        "void main() {\n"
        "    camera_relative_normal = global_orientation * orientation * normal;\n"
        "    camera_relative_position = global_position + global_orientation * (position + orientation * vpos);\n"
        "    gl_Position = perspective * vec4(camera_relative_position, 1.0f);\n"
        "}\n";

    // Fragment shader description:
    // Draws colour based on colour mask.
    // Faces directly facing the camera are drawn brighter than those that are barely facing the camera.
    const char* fshader_string =
        "#version 140\n"
        "uniform vec3 colour_mask;\n"
        "in vec3 camera_relative_normal;\n"
        "in vec3 camera_relative_position;\n"
        "out vec4 colour;\n"
        "void main() {\n"
        "    float angular_component = -dot(normalize(camera_relative_position), normalize(camera_relative_normal));\n"
        "    float intensity = 0.5f + 0.5f * angular_component;\n"
        "    colour = vec4(intensity * colour_mask, 1.0f);\n"
        "}\n";

    // Compile and link the shader program
    shader_program = ShaderProgram(vshader_string, fshader_string);

    // Make a perspective matrix with near plane at 0.1f, far at 100.0f, FOV of 1 rad (57 deg).
    make_perspective_matrix(perspective_matrix, 0.1f, 100.0f, 1.0f, float(width) / float(height));
}

RenderContext::~RenderContext()
{
    glfwTerminate();
}

std::size_t RenderContext::load_object(const Vec3* positions, const Vec3* normals, std::size_t count)
{
    // Create VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and fill VBO with position and normal data
    GLuint pos_vbo;
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * count, positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   // Location for position in shader program
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    // Create and fill VBO for normal data
    GLuint normal_vbo;
    glGenBuffers(1, &normal_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * count, normals, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);   // Location for normal in shader program
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    // Keep track of the newly created object.
    std::size_t object_id = objects.size();
    objects.emplace_back();

    objects.back().pos_vbo = pos_vbo;
    objects.back().normal_vbo = normal_vbo;
    objects.back().vao = vao;
    objects.back().num_vertices = count;

    return object_id;
}

void RenderContext::draw_object(std::size_t object_id, const Vec3& position, const float* orientation, bool selected, bool colliding, const Vec3& global_position, const float* global_orientation)
{
    // Make sure the shader is bound
    glUseProgram(shader_program.program);

    const RenderObject& object = objects[object_id];

    glBindVertexArray(object.vao);

    // Pass the perspective matrix to the shader
    GLint location = glGetUniformLocation(shader_program.program, "perspective");
    glUniformMatrix4fv(location, 1, GL_TRUE, perspective_matrix);

    // Pass the object orientation matrix to the shader
    location = glGetUniformLocation(shader_program.program, "orientation");
    glUniformMatrix3fv(location, 1, GL_TRUE, orientation);

    // Pass the object position vector to the shader
    location = glGetUniformLocation(shader_program.program, "position");
    glUniform3f(location, position.x, position.y, position.z);
    
    // Pass the colour mask to the shader (which determines the object colour).
    location = glGetUniformLocation(shader_program.program, "colour_mask");

    // Base colour is grey (xyz = rgb)
    Vec3 colour_mask(0.5f, 0.5f, 0.5f);
    if (selected)
    {
        // Selected objects have a blue component
        colour_mask.z = 1.0f;
    }
    if (colliding)
    {
        // Intersecting objects have a stronger red component
        colour_mask.x = 1.0f;
    }
    glUniform3f(location, colour_mask.x, colour_mask.y, colour_mask.z);

    // Pass global position to the shader
    location = glGetUniformLocation(shader_program.program, "global_position");
    glUniform3f(location, global_position.x, global_position.y, global_position.z);

    // Pass global orientation to the shader
    location = glGetUniformLocation(shader_program.program, "global_orientation");
    glUniformMatrix3fv(location, 1, GL_TRUE, global_orientation);

    // Draw the object
    glDrawArrays(GL_TRIANGLES, 0, object.num_vertices);
}

// The GLFW window is needed for event handling
GLFWwindow* RenderContext::get_glfw_window()
{
    return window;
}

// ShaderProgram constructor takes the strings of a vertex and fragment shader, and compiles and links them
// into a shader program.
RenderContext::ShaderProgram::ShaderProgram(const char* vshader_string, const char* fshader_string)
{
    vertex_shader = compile_shader(vshader_string, GL_VERTEX_SHADER);
    fragment_shader = compile_shader(fshader_string, GL_FRAGMENT_SHADER);
    program = link_shader_program(vertex_shader, fragment_shader);
}

// Returns shader object handle
GLuint compile_shader(const char* source, GLenum shader_type)
{
    // Create the shader object
    GLuint shader = glCreateShader(shader_type);

    // Pass the shader string to the shader object
    glShaderSource(shader, 1, &source, nullptr);

    //Compile the shader
    glCompileShader(shader);

    // Check if there was an error and if so, show the error message
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
    // Create the shader program object
    GLuint program = glCreateProgram();

    // Attach the two compiled shaders to the program
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);

    // Link the program
    glLinkProgram(program);

    // Check if there was an error and if so, show the error message
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

// This returns a perspective matrix in data (row-major, 4x4)
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
