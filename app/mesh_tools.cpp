#include "mesh_tools.hpp"
#include "math.hpp"
#include <cstddef>
#include <vector>

namespace demo::mesh {

void compute_normals(const std::vector<demo::math::Vec3>& triangles,
                     std::vector<demo::math::Vec3>& normals)
{
    for (std::size_t i = 0; i < triangles.size(); i += 3)
    {
        demo::math::Vec3 normal = demo::math::cross(triangles[i+1] - triangles[i], triangles[i+2] - triangles[i]);
        normal.normalize();
        
        // Add three copies of the computed normal;
        normals.resize(normals.size() + 3, normal);
    }
}

}
