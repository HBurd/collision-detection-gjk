#include "load_mesh.hpp"
#include "mesh_tools.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>  // using strtoul instead of stoul, since no exceptions
#include <cctype>
#include <cstring>

namespace demo::mesh {

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

// Returns false if the line is not valid
bool off_parse_vertex(const std::string& line, std::vector<demo::math::Vec3>& vertices)
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
            return false;
        }

        const char* y_start = end;
        float y = strtof(y_start, &end);
        if (end == y_start)
        {
            // This means no conversion could be performed
            return false;
        }

        const char* z_start = end;
        float z = strtof(z_start, &end);
        if (end == z_start)
        {
            // This means no conversion could be performed
            return false;
        }

        vertices.emplace_back(x, y, z);
    }

    return true;
}

// Returns false if the line is not valid
bool off_parse_face(const std::string& line,
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
            std::cerr << "Only faces with 3 vertices are supported.\n";
            return false;
        }

        demo::math::Vec3 vertices_to_add[3];

        for (std::size_t i = 0; i < 3; ++i)
        {
            const char* v_start = end;
            std::size_t v_idx = strtoul(v_start, &end, 10);
            if (end == v_start)
            {
                // This means no conversion could be performed
                std::cerr << "Unable to read vertex number.\n";
                return false;
            }
            if (v_idx >= vertices.size())
            {
                // This means v_idx doesn't represent a valid vertex
                std::cerr << "Vertex " << v_idx << " is not a valid vertex.\n";
                return false;
            }

            vertices_to_add[i] = vertices[v_idx];
        }

        // If control reaches here, parsing was successful
        for (std::size_t i = 0; i < 3; ++i)
        {
            triangles.push_back(vertices_to_add[i]);
        }
    }

    return true;
}

void load_off(const char* filename,
    std::vector<demo::math::Vec3>& vertices,
    std::vector<demo::math::Vec3>& triangles,
    std::vector<demo::math::Vec3>& normals)
{
    vertices.clear();
    triangles.clear();
    normals.clear();

    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Could not open file " << filename << ".\n";
        return;
    }

    std::string line;

    std::size_t line_num = 1;

    std::size_t vertex_count = 0;
    std::size_t face_count = 0;
    OffParseState parse_state = OffParseState::FirstLine;

    bool error = false;
    
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
                    // Files without the optional OFF are not supported
                    std::cerr << "Line " << line_num << ": The first line must contain the characters OFF.\n";
                    error = true;
                }
                break;
            case OffParseState::Counts:
                if (off_parse_counts(line, vertex_count, face_count))
                {
                    if (vertex_count == 0 || face_count == 0)
                    {
                        // This means the line couldn't be parsed
                        std::cerr << "Line " << line_num << ": Unable to read vertex count or face count.\n";
                        error = true;
                    }
                    else
                    {
                        vertices.reserve(vertex_count);
                        triangles.reserve(3*face_count); // explicitly only support 3 vertices per face
                        parse_state = OffParseState::Vertices;
                    }
                }
                break;
            case OffParseState::Vertices:
                if (!off_parse_vertex(line, vertices))
                {
                    std::cerr << "Line " << line_num << ": Unable to read vertex.\n";
                    error = true;
                }
                if (vertices.size() == vertex_count)
                {
                    parse_state = OffParseState::Faces;
                }
                break;
            case OffParseState::Faces:
                if (!off_parse_face(line, vertices, triangles))
                {
                    std::cerr << "Line " << line_num << ": Unable to read face.\n";
                    error = true;
                }
                if (triangles.size() == 3*face_count)
                {
                    parse_state = OffParseState::Done;
                }
                break;
            case OffParseState::Done:
            default:
                break;
        }

        if (error)
        {
            std::cerr << "Error reading OFF file.\n";
            break;
        }

        ++line_num;
    }

    if (vertices.size() != vertex_count || triangles.size() != 3*face_count)
    {
        // This means the file wasn't parsed properly, so return failure
        vertices.clear();
        triangles.clear();
    }
    else
    {
        // Compute normals
        compute_normals(triangles, normals);
    }
}

}
