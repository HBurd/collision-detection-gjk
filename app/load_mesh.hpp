#ifndef LOAD_MESH_HPP
#define LOAD_MESH_HPP

#include <vector>
#include <string>
#include <cstddef>
#include "math.hpp"

namespace demo::mesh {

// These are internal, but the declarations are here for testing
bool off_parse_counts(const std::string& line, std::size_t& vertex_count, std::size_t& face_count);
bool off_parse_vertex(const std::string& line, std::vector<demo::math::Vec3>& vertices);
bool off_parse_face(const std::string& line,
                    const std::vector<demo::math::Vec3>& vertices,
                    std::vector<demo::math::Vec3>& triangles);
// End internal

void load_off(const char* filename,
              std::vector<demo::math::Vec3>& vertices,
              std::vector<demo::math::Vec3>& triangles);

}

#endif
