#include "load_mesh.hpp"
#include <fstream>
#include <string>
#include <cstdlib>  // using strtoul instead of stoul, since no exceptions
#include <cctype>

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

bool off_parse_vertex(const std::string& line, std::vector<demo::math::Vec3>& vertices)
{
    return false;
}

bool off_parse_face(const std::string& line,
                    const std::vector<demo::math::Vec3>& vertices,
                    std::vector<demo::math::Vec3>& triangles)
{
    return false;
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
