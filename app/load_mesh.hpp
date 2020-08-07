#ifndef LOAD_MESH_HPP
#define LOAD_MESH_HPP

#include <vector>
#include <string>
#include <cstddef>
#include "math.hpp"

namespace demo::mesh {

// These are internal, but the declarations are here for testing
bool off_parse_counts(const std::string& line, std::size_t& vertex_count, std::size_t& face_count);
void off_parse_vertex(const std::string& line, std::vector<demo::math::Vec3>& vertices);
void off_parse_face(const std::string& line,
                    const std::vector<demo::math::Vec3>& vertices,
                    std::vector<demo::math::Vec3>& triangles);
// End internal

// Loads a mesh from a file and computes normals if necessary
// (it might not be necessary in the OBJ case).
// Right now this only supports OFF, but it may support other
// filetypes (e.g. OBJ) in the future.
void load_mesh(const char* filename,
               std::vector<demo::math::Vec3>& vertices,
               std::vector<demo::math::Vec3>& triangles,
               std::vector<demo::math::Vec3>& normals);

void load_off(const char* filename,
              std::vector<demo::math::Vec3>& vertices,
              std::vector<demo::math::Vec3>& triangles);

}

#endif
