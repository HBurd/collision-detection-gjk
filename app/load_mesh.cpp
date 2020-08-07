#include "load_mesh.hpp"
#include "mesh_tools.hpp"
#include <fstream>
#include <string>
#include <cstdlib>  // using strtoul instead of stoul, since no exceptions
#include <cctype>
#include <cstring>

namespace demo::mesh {

void load_mesh(const char* filename,
               std::vector<demo::math::Vec3>& vertices,
               std::vector<demo::math::Vec3>& triangles,
               std::vector<demo::math::Vec3>& normals)
{
    // Find the last occurrence of '.' for file extension
    const char* extension = strrchr(filename, '.');
    if (!extension)
    {
        // No filename so the file type can't be determined
        return;
    }

    if (strcmp(extension, ".off") == 0)
    {
        // Load vertices and triangles
        load_off(filename, vertices, triangles);

        // Compute normals
        compute_normals(triangles, normals);
    }
    else if (strcmp(extension, ".obj") == 0)
    {
        // obj not supported yet
        return;
    }
}


enum class OffParseState
{
    FirstLine,
    Counts,
    Vertices,
    Faces,
    Done
};

// Returns index of first non-whitespace character
std::size_t skip_whitespace(const char* str)
{
    std::size_t idx = 0;

    // Skip to the first non-whitespace character in the line
    while (isspace(str[idx]))
    {
        ++idx;
    }

    return idx;
}

// Returns false for blank lines or comments.
// If either vertex_count or face_count is set to zero, the line couldn't be parsed (or it is zero).
// This ambiguity is irrelevant since both are failure cases.
bool off_parse_counts(const std::string& line, std::size_t& vertex_count, std::size_t& face_count)
{
    std::size_t line_idx = skip_whitespace(line.c_str());

    // Only parse a number if a non-comment character was found
    if (line[line_idx] && line[line_idx] != '#')
    {
        char* end;

        const char* vertex_count_start = line.c_str() + line_idx;
        vertex_count = strtoul(vertex_count_start, &end, 10);

        const char* face_count_start = end;
        face_count = strtoul(face_count_start, &end, 10);

        // Skip the edge count because it is not needed.
        return true;
    }

    return false;
}

// If the line isn't a valid vertex, this function will silently fail by
// not adding a vertex to vertices. This is also the behaviour for valid
// lines such as whitespace or comments.
void off_parse_vertex(const std::string& line, std::vector<demo::math::Vec3>& vertices)
{
    std::size_t line_idx = skip_whitespace(line.c_str());

    // Only parse a number if a non-comment character was found
    if (line[line_idx] && line[line_idx] != '#')
    {
        char* end;

        const char* x_start = line.c_str() + line_idx;
        float x = strtof(x_start, &end);
        if (end == x_start)
        {
            // This means no conversion could be performed
            return;
        }

        const char* y_start = end;
        float y = strtof(y_start, &end);
        if (end == y_start)
        {
            // This means no conversion could be performed
            return;
        }

        const char* z_start = end;
        float z = strtof(z_start, &end);
        if (end == z_start)
        {
            // This means no conversion could be performed
            return;
        }

        vertices.emplace_back(x, y, z);
    }
}

// If the line doesn't validly describe a triangle, this function fails
// silently. This is also the behaviour for comments and whitespace.
void off_parse_face(const std::string& line,
                    const std::vector<demo::math::Vec3>& vertices,
                    std::vector<demo::math::Vec3>& triangles)
{
    std::size_t line_idx = skip_whitespace(line.c_str());

    // Only parse a number if a non-comment character was found
    if (line[line_idx] && line[line_idx] != '#')
    {
        char* end;

        const char* vertex_count_start = line.c_str() + line_idx;
        std::size_t vertex_count = strtoul(vertex_count_start, &end, 10);

        // Only triangle-faces are supported
        if (vertex_count != 3)
        {
            return;
        }

        demo::math::Vec3 vertices_to_add[3];

        for (std::size_t i = 0; i < 3; ++i)
        {
            const char* v_start = end;
            std::size_t v_idx = strtoul(v_start, &end, 10);
            if (end == v_start)
            {
                // This means no conversion could be performed
                return;
            }
            if (v_idx >= vertices.size())
            {
                // This means v_idx doesn't represent a valid vertex
                return;
            }

            vertices_to_add[i] = vertices[v_idx];
        }

        // If control reaches here, parsing was successful
        for (std::size_t i = 0; i < 3; ++i)
        {
            triangles.push_back(vertices[i]);
        }
    }
}

void load_off(const char* filename,
    std::vector<demo::math::Vec3>& vertices,
    std::vector<demo::math::Vec3>& triangles)
{
    std::ifstream file(filename);
    std::string line;

    std::size_t vertex_count = 0;
    std::size_t face_count = 0;
    OffParseState parse_state = OffParseState::FirstLine;
    
    while (getline(file, line) && parse_state != OffParseState::Done)
    {
        switch (parse_state)
        {
            case OffParseState::FirstLine:
                if (line == "OFF")
                {
                    parse_state = OffParseState::Counts;
                }
                else
                {
                    // TODO: Should files without the optional OFF be supported?
                    // For now return failure
                    return;
                }
                break;
            case OffParseState::Counts:
                if (off_parse_counts(line, vertex_count, face_count))
                {
                    if (vertex_count == 0 || face_count == 0)
                    {
                        // This means the line couldn't be parsed
                    }

                    vertices.reserve(vertex_count);
                    triangles.reserve(3*face_count); // explicitly only support 3 vertices per face
                    parse_state = OffParseState::Vertices;
                }
                break;
            case OffParseState::Vertices:
                off_parse_vertex(line, vertices);
                if (vertices.size() == vertex_count)
                {
                    parse_state = OffParseState::Faces;
                }
                break;
            case OffParseState::Faces:
                off_parse_face(line, vertices, triangles);
                if (triangles.size() == 3*face_count)
                {
                    parse_state = OffParseState::Done;
                }
                break;
            case OffParseState::Done:
            default:
                break;
        }
    }

    if (vertices.size() != vertex_count || triangles.size() != 3*face_count)
    {
        // This means the file wasn't parsed properly, so return failure
        vertices.clear();
        triangles.clear();
    }
}

}
