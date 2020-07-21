#include "gjk.hpp"
#include "math.hpp"

#include <array>
#include <iostream>
#include <GL/glew.h>
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

int main()
{
    assert(glfwInit() != -1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SENG 475 Project Demo", nullptr, nullptr);
    assert(window);
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    assert(glewInit() == GLEW_OK);


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

    GLuint cube_buf;
    glGenBuffers(1, &cube_buf);

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

    glBindBuffer(GL_ARRAY_BUFFER, cube_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * cube_vertices.size(), cube_vertices.data(), GL_STATIC_DRAW);

    GLuint cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0);

    GLuint cube_element_buf;
    glGenBuffers(1, &cube_element_buf);

    std::array<uint32_t, 36> cube_elements = {
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * cube_elements.size(), cube_elements.data(), GL_STATIC_DRAW);

    const char* vshader_string =
        "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "void main() {\n"
        "    gl_Position = vec4(pos.x, pos.y, 0.0f, 1.0f);\n"
        "}\n";
    GLuint vshader = compile_shader(vshader_string, GL_VERTEX_SHADER);

    const char* fshader_string =
        "#version 330 core\n"
        "out vec4 colour;\n"
        "void main() {\n"
        "    colour = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n"
        "}\n";
    GLuint fshader = compile_shader(fshader_string, GL_FRAGMENT_SHADER);

    GLuint shader_program = link_shader_program(vshader, fshader);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buf);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
