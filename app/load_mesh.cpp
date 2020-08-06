#include "load_mesh.h"
#include <fstream>
#include <string>

namespace demo::mesh {

enum class OffParseState
{
    FirstLine,
    Counts,
    Vertices,
    Faces,
    Done
};

void load_off(const char* filename,
    std::vector<demo::math::Vec3>& vertices,
    std::vector<demo::math::Vec3>& triangles)
{
    std::ifstream file(filename);
    std::string line;

    std::size_t vertex_count;
    std::size_t face_count;
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
                if (parse_off_counts(line, vertex_count, face_count))
                {
                    vertices.reserve(vertex_count);
                    triangles.reserve(3*face_count); // explicitly only support 3 vertices per face
                    parse_state = OffParseState::Vertices;
                }
                break;
            case OffParseState::Vertices:
                parse_off_vertex(line, vertices);
                if (vertices.size() == vertex_count)
                {
                    parse_state = OffParseState::Faces;
                }
                break;
            case OffParseState::Faces:
                parse_off_face(line, vertices, triangles);
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
