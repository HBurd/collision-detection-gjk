#ifndef GJK_HPP
#define GJK_HPP

#include <cstddef>
#include <cassert>

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

    }

    // We are treating resting contact as an intersection
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
