#ifndef LOAD_MESH_H
#define LOAD_MESH_H

#include <vector>
#include "math.hpp"

namespace demo::mesh {

void load_off(const char* filename,
    std::vector<demo::math::Vec3>& vertices,
    std::vector<demo::math::Vec3>& triangles);

}
