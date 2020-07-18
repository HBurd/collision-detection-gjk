#include "gjk.hpp"
#include "math.hpp"
#include <cassert>

using namespace demo::math;

void assert_equal(Vec3 v1, Vec3 v2)
{
    float epsilon = 0.001f;
    assert(v1.x - v2.x < epsilon && v1.y - v2.y < epsilon && v1.z - v2.z < epsilon);
}

void test_simplex1_dir()
{
    Vec3 simplex(1.0f, 2.0f, 1.0f);
    Vec3 d;
    geometry::simplex1_dir(&simplex, d);
    assert_equal(d, Vec3(-1.0f, -2.0f, -1.0f));
}

void test_simplex2_dir()
{
    Vec3 simplex1[2] = {
        Vec3(0.0f, -1.0f, 0.0f),
        Vec3(0.1f, 1.0f, 0.0f)
    };
    Vec3 expected1(-2.0f, 0.1f, 0.0f);
    Vec3 d;

    geometry::simplex2_dir(simplex1, d);
    
    assert(dot(d, expected1) - d.mag()*expected1.mag() < 0.0001f);

    Vec3 simplex2[2] = {
        Vec3(0.0f, -1.0f, 0.0f),
        Vec3(-0.1f, 1.0f, 0.0f)
    };
    Vec3 expected2(2.0f, 0.1f, 0.0f);

    geometry::simplex2_dir(simplex2, d);

    assert(dot(d, expected2) - d.mag()*expected2.mag() < 0.0001f);
}

void test_simplex3_dir()
{

}

void test_simplex4_dir()
{

}

// This tests functions that aren't part of the interface.
void test_gjk_internals()
{
    test_simplex1_dir();
    test_simplex2_dir();
    test_simplex3_dir();
    test_simplex4_dir();
}


int main()
{
    test_gjk_internals();

    return 0;
}
