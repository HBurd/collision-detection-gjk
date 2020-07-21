#ifndef GJK_HPP
#define GJK_HPP

#include <cstddef>
#include <cassert>
#include <algorithm>

namespace geometry
{

    template <class Vec3, class Data>
    using SupportFn = Vec3 (*) (Vec3 dir, const Data& data);

    template <class Vec3>
    decltype(Vec3::x) dot(const Vec3& l, const Vec3& r)
    {
        return l.x*r.x + l.y*r.y + l.z*r.z;
    }

    template <class Vec3>
    Vec3 cross(const Vec3& l, const Vec3& r)
    {
        return Vec3(
            l.y*r.z - l.z*r.y,
            l.z*r.x - l.x*r.z,
            l.x*r.y - l.y*r.x
        );
    }

    template <class Vec3>
    void simplex1_dir(Vec3* simplex, Vec3& d)
    {
        // The simplex is a single point
        d = -*simplex;
    }

    template <class Vec3>
    void simplex2_dir(Vec3* simplex, Vec3& d)
    {
        // The simplex is 2 points.
        // The way this is called, the closest point to the simplex can only
        // be on the line. So d will be normal t the line.

        // This function returns the vector on the plane containing the simplex and the origin,
        // in the direction pointing "closest" to the origin while being normal to the simplex.
        // There are 2 directions normal to the simplex in this plane

        d = cross(simplex[1] - simplex[0], cross(simplex[1] - simplex[0], simplex[0]));
    }

    template <class Vec3>
    void simplex3_dir(Vec3* simplex, Vec3& d)
    {
        /*  The simplex is 3 points, a triangle

              A         .
                    .
          . . . .
               |\
               | \      F
           B   |  \
               |G/H\          .
               |    \     .
          . . .|_____\.
               .     .
           C   .  D  .    E
               .     .

            The vertices by areas A, C and E are the first, second and third vertices
            added to the simplex respectively (and call them a, b and c). The way the
            GJK algorithm works imposes constraints on where the origin can be relative
            to these vertices. This allows several cases to be eliminated in this function.
            In particular, the only regions which can contain the origin are D, F, G and H.
        */

        using Real = decltype(Vec3::x);

        // Looking down at counter-clockwise triangle, this is pointing up
        Vec3 triangle_normal = cross(simplex[1] - simplex[0], simplex[2] - simplex[0]);

        Real f_plane(dot(cross(triangle_normal, simplex[2] - simplex[0]), simplex[0]));
        Real d_plane(dot(cross(triangle_normal, simplex[1] - simplex[2]), simplex[2]));

        // TODO: decide on < vs <=

        if (f_plane * d_plane < Real(0))
        {
            // Origin is in region D or F

            if (f_plane < Real(0))
            {
                // Origin is in region F
                // We can discard simplex[1]
                simplex[1] = simplex[2];
                simplex2_dir(simplex, d);
            }
            else
            {
                // Origin is in region D
                // We can discard simplex[0]
                simplex[0] = simplex[2];
                simplex2_dir(simplex, d);
            }
        }
        else
        {
            // Origin is in region G or H
            if (dot(triangle_normal, simplex[0]) < Real(0))
            {
                // Origin is above the triangle
                d = triangle_normal;

                // Make the search direction opposite the triangle normal,
                // so the 4-simplex is always consistently constructed.
                std::swap(simplex[1], simplex[2]);
            }
            else
            {
                // Origin is below triangle
                d = -triangle_normal;
            }
        }
    }

    template <class Vec3>
    bool simplex4_dir(Vec3* simplex, Vec3& d)
    {
        using Real = decltype(Vec3::x);

        /* Triangle 0 vertices are 0, 1, 2.
           Triangle 1 vertices are 1, 0, 3.
           Triangle 2 vertices are 2, 3, 0.
           Triangle 3 vertices are 3, 2, 1.
           All triangles are counter-clockwise when looking from the outside.
           All normals are pointing outward.

           We know beforehand that the origin is on the negative side of triangle 0,
           so it does not need to be tested. The origin is also not closest to vertex
           3, so it need not be tested. There may be several other cases that
           can be eliminated, but I have not figured out which ones yet.
        */

        //Vec3 triangle0_normal = cross(simplex[1] - simplex[0], simplex[2] - simplex[0]);
        Vec3 triangle1_normal = cross(simplex[0] - simplex[1], simplex[3] - simplex[1]);
        Vec3 triangle2_normal = cross(simplex[3] - simplex[2], simplex[0] - simplex[2]);
        Vec3 triangle3_normal = cross(simplex[2] - simplex[3], simplex[1] - simplex[3]);

        // Pointing toward face, away from edge
        Vec3 triangle1_edge12_boundary_normal = cross(triangle1_normal, simplex[3] - simplex[0]);
        Vec3 triangle1_edge13_boundary_normal = cross(triangle1_normal, simplex[1] - simplex[3]);
        Vec3 triangle2_edge21_boundary_normal = cross(triangle2_normal, simplex[0] - simplex[3]);
        Vec3 triangle2_edge23_boundary_normal = cross(triangle2_normal, simplex[3] - simplex[2]);
        Vec3 triangle3_edge31_boundary_normal = cross(triangle3_normal, simplex[3] - simplex[1]);
        Vec3 triangle3_edge32_boundary_normal = cross(triangle3_normal, simplex[2] - simplex[3]);

        // If positive, the origin is on the negative side of the plane
        Real side_of_triangle1 = dot(triangle1_normal, simplex[0]);
        Real side_of_triangle2 = dot(triangle2_normal, simplex[3]);
        Real side_of_triangle3 = dot(triangle3_normal, simplex[2]);

        Real  side_of_triangle1_edge12_boundary = dot(triangle1_edge12_boundary_normal, simplex[0]);
        Real  side_of_triangle1_edge13_boundary = dot(triangle1_edge13_boundary_normal, simplex[3]);
        Real  side_of_triangle2_edge21_boundary = dot(triangle2_edge21_boundary_normal, simplex[3]);
        Real  side_of_triangle2_edge23_boundary = dot(triangle2_edge23_boundary_normal, simplex[2]);
        Real  side_of_triangle3_edge31_boundary = dot(triangle3_edge31_boundary_normal, simplex[1]);
        Real  side_of_triangle3_edge32_boundary = dot(triangle3_edge32_boundary_normal, simplex[3]);

        if (side_of_triangle1 < Real(0))
        {
            if (side_of_triangle1_edge12_boundary >= Real(0))
            {
                // Closest to edge12 or triangle2
                if (side_of_triangle2_edge21_boundary < 0)
                {
                    // Closest to triangle2
                    d = triangle2_normal;
                }
                else
                {
                    // Closest to edge12
                    d = cross(cross(simplex[3] - simplex[0], -simplex[0]), simplex[3] - simplex[0]);
                }
            }
            else if (side_of_triangle1_edge13_boundary >= Real(0))
            {
                // Closest to edge13 or triangle3
                if (side_of_triangle2_edge23_boundary < 0)
                {
                    // Closest to triangle3
                    d = triangle3_normal;
                }
                else
                {
                    // Closest to edge13
                    d = cross(cross(simplex[1] - simplex[3], -simplex[3]), simplex[1] - simplex[3]);
                }
            }
            else
            {
                // Closest to triangle1
                d = triangle1_normal;
            }
        }
        else if (side_of_triangle2 < Real(0))
        {
            if (side_of_triangle2_edge23_boundary >= Real(0))
            {
                // Closest to edge23 or triangle3
                if (side_of_triangle3_edge32_boundary < Real(0))
                {
                    // Closest to triangle3
                    d = triangle3_normal;
                }
                else
                {
                    // Closest to edge23
                    d = cross(cross(simplex[3] - simplex[2], -simplex[2]), simplex[3] - simplex[2]);
                }
            }
            else if (side_of_triangle2_edge21_boundary >= Real(0))
            {
                // Closest to edge12, since closest to triangle1 case is already ruled out
                d = cross(cross(simplex[3] - simplex[0], -simplex[0]), simplex[3] - simplex[0]);
            }
            else
            {
                // Closest to triangle2
                d = triangle2_normal;
            }
        }
        else if (side_of_triangle3 < Real(0))
        {
            if (side_of_triangle3_edge32_boundary >= Real(0))
            {
                // Closest to edge32, since closest to triangle2 case already ruled out
                d = cross(cross(simplex[3] - simplex[2], -simplex[2]), simplex[3] - simplex[2]);
            }
            else if (side_of_triangle3_edge31_boundary >= Real(0))
            {
                // Closest to edge31, since closest to triangle2 case already ruled out
                d = cross(cross(simplex[1] - simplex[3], -simplex[3]), simplex[1] - simplex[3]);
            }
            else
            {
                // Closest to triangle3
                d = triangle3_normal;
            }
        }
        else
        {
            // Origin is contained in the triangle
            // TODO: Should d be set here?
            return true;
        }

        return false;
    }

    // TODO: are we treating resting contact as an intersection?
    template <class Vec3, class Data1, class Data2>
    bool intersect_gjk(SupportFn<Vec3, Data1> support1, const Data1& data1, SupportFn<Vec3, Data2> support2, const Data2& data2)
    {
        using Real = decltype(Vec3::x);

        // Starting direction is arbitrary
        Vec3 d = Vec3(1.0f, 0.0f, 0.0f);

        Vec3 simplex_points[4] = {};
        std::size_t simplex_size = 0;

        bool intersection = false;
        do
        {
            Vec3 point = support1(d, data1) - support2(-d, data2);
            if (dot(point, d) < Real(0))
            {
                // Furthest point along d is not past the origin, so there is no intersection
                return false;
            }
            simplex_points[simplex_size++] = point;

            switch (simplex_size)
            {
            case 1:
                simplex1_dir(simplex_points, d);
                break;
            case 2:
                simplex2_dir(simplex_points, d);
                break;
            case 3:
                simplex3_dir(simplex_points, d);
                break;
            case 4:
                intersection = simplex4_dir(simplex_points, d);
                break;
            default:
                // Impossible case
                assert(false);
            }
        } while (!intersection);

        return true;
    }

}

#endif
