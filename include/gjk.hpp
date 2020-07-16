#ifndef GJK_HPP
#define GJK_HPP

#include <cstddef>
#include <cassert>

namespace geometry
{

    template <class Vec3, class Data>
    using SupportFn = Vec3 (*) (Vec3 dir, const Data& data);

    template<class Vec3>
    decltype(Vec3::x) dot(const Vec3& v1, const Vec3& v2)
    {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }

    template <class Vec3>
    void simplex1_dir(Vec3* simplex, Vec3& d)
    {

    }

    template <class Vec3>
    void simplex2_dir(Vec3* simplex, Vec3& d)
    {

    }

    // TODO: can this return bool so this can be used in 2D also?
    template <class Vec3>
    void simplex3_dir(Vec3* simplex, Vec3& d)
    {

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
            if (dot(point, d) < 0)
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
