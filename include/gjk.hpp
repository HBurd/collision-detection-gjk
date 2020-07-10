#ifndef GJK_HPP
#define GJK_HPP

namespace geometry
{

template<class Vec3, class Data>
using SupportFn = Vec3 (*) (Vec3 dir, const Data& data);

template<class Vec3, class Data1, class Data2, class Real = typename Vec3::real_type>
bool intersect_gjk(SupportFn<Vec3, Data1> support1, const Data1& data1, SupportFn<Vec3, Data2> support2, const Data2& data2)
{

}

}

#endif
