#ifndef MESH_TOOLS
#define MESH_TOOLS

#include <vector>
#include "math.hpp"

namespace demo::mesh {

// Computes vertex normals for an array of vertices, representing triangles.
// Assumes counter-clockwise is outward-facing.
// Output is a vector of the same length of triangles, with normals corresponding
// to each vertex.
void compute_normals(const std::vector<demo::math::Vec3>& triangles,
                     std::vector<demo::math::Vec3>& normals);

}

#endif
