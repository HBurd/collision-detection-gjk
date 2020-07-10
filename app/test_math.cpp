#include "math.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using std::cout;
using std::endl;

using namespace demo::math;

bool are_equal(const Vec3& a, const Vec3& b)
{
    const float epsilon = 0.001;

    return (abs(a.x - b.x) < epsilon)
        && (abs(a.y - b.y) < epsilon)
        && (abs(a.z - b.z) < epsilon);
}

bool are_equal(float a, float b)
{
    const float epsilon = 0.001;

    return abs(a - b) < epsilon;
}

bool are_equal(Mat3 a, Mat3 b)
{
    const float epsilon = 0.001;

    return abs(a[0][0] - b[0][0]) < epsilon
        && abs(a[0][1] - b[0][1]) < epsilon
        && abs(a[0][2] - b[0][2]) < epsilon

        && abs(a[1][0] - b[1][0]) < epsilon
        && abs(a[1][1] - b[1][1]) < epsilon
        && abs(a[1][2] - b[1][2]) < epsilon

        && abs(a[2][0] - b[2][0]) < epsilon
        && abs(a[2][1] - b[2][1]) < epsilon
        && abs(a[2][2] - b[2][2]) < epsilon;
}

int main()
{
    // Vector constructors
    {
        Vec3 zero;
        assert(zero.x == 0.0f);
        assert(zero.y == 0.0f);
        assert(zero.z == 0.0f);

        Vec3 nonzero(5.0f, 6.0f, 7.0f);
        assert(nonzero.x == 5.0f);
        assert(nonzero.y == 6.0f);
        assert(nonzero.z == 7.0f);
    }

    // Unary operators
    {
        Vec3 a(1.0f, 2.0f, 3.0f);
        Vec3 b(-1.0f, -2.0f, -3.0f);

        assert(are_equal(+a, a));
        assert(are_equal(-a, b));
    }

    // Binary operators
    {
        Vec3 a(1.0f, 2.0f, 3.0f);
        assert(are_equal(a + a, Vec3(2.0f, 4.0f, 6.0f)));
        assert(are_equal(a - a, Vec3()));
    }

    // Compound assignment
    {
        Vec3 a(1.0f, 1.0f, 1.0f);
        Vec3 b(2.0f, 3.0f, 4.0f);

        assert(are_equal(a += b, Vec3(3.0f, 4.0f, 5.0f)));
        assert(are_equal(a, Vec3(3.0f, 4.0f, 5.0f)));

        assert(are_equal(a -= b, Vec3(1.0f, 1.0f, 1.0f)));
        assert(are_equal(a, Vec3(1.0f, 1.0f, 1.0f)));
    }

    // Magnitude
    {
        Vec3 a(1.0f, 2.0f, 3.0f);
        assert(are_equal(a.sq_mag(), 1.0f + 4.0f + 9.0f));
        assert(are_equal(a.mag(), sqrtf(1.0f + 4.0f + 9.0f)));
    }

    // Basic matrix constructors, operator[], row, col
    {
        Mat3 m(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
        for (int i = 0; i < 9; ++i)
        {
            assert(are_equal(i * 1.0f, m.m[i/3][i % 3]));
            assert(are_equal(i * 1.0f, m[i/3][i % 3]));
        }

        assert(are_equal(m.row(0), Vec3(0.0f, 1.0f, 2.0f)));
        assert(are_equal(m.row(1), Vec3(3.0f, 4.0f, 5.0f)));
        assert(are_equal(m.row(2), Vec3(6.0f, 7.0f, 8.0f)));

        assert(are_equal(m.col(0), Vec3(0.0f, 3.0f, 6.0f)));
        assert(are_equal(m.col(1), Vec3(1.0f, 4.0f, 7.0f)));
        assert(are_equal(m.col(2), Vec3(2.0f, 5.0f, 8.0f)));

        Mat3 id(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        assert(are_equal(id, Mat3()));
        assert(are_equal(id, Mat3::Identity()));

        Mat3 zero(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        assert(are_equal(zero, Mat3::Zero()));

        assert(are_equal(Mat3::FromColumns(Vec3(1.0f, 2.0f, 3.0f), Vec3(4.0f, 5.0f, 6.0f), Vec3(7.0f, 8.0f, 9.0f)), Mat3(1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f)));
    }

    // set_row, set_col
    {
        Mat3 m = Mat3::Zero();
        Vec3 v(1.0f, 2.0f, 3.0f);

        m.set_row(0, v);
        assert(are_equal(m.row(0), v));
        assert(are_equal(m.row(1), Vec3()));
        assert(are_equal(m.row(2), Vec3()));
        m.set_row(0, Vec3());

        m.set_row(1, v);
        assert(are_equal(m.row(0), Vec3()));
        assert(are_equal(m.row(1), v));
        assert(are_equal(m.row(2), Vec3()));
        m.set_row(1, Vec3());

        m.set_row(2, v);
        assert(are_equal(m.row(0), Vec3()));
        assert(are_equal(m.row(1), Vec3()));
        assert(are_equal(m.row(2), v));
        m.set_row(2, Vec3());

        m.set_col(0, v);
        assert(are_equal(m.col(0), v));
        assert(are_equal(m.col(1), Vec3()));
        assert(are_equal(m.col(2), Vec3()));
        m.set_col(0, Vec3());

        m.set_col(1, v);
        assert(are_equal(m.col(0), Vec3()));
        assert(are_equal(m.col(1), v));
        assert(are_equal(m.col(2), Vec3()));
        m.set_col(1, Vec3());

        m.set_col(2, v);
        assert(are_equal(m.col(0), Vec3()));
        assert(are_equal(m.col(1), Vec3()));
        assert(are_equal(m.col(2), v));
        m.set_col(2, Vec3());
    }

    // Matrix multiplication
    {
        Mat3 m1(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        Mat3 m2(10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f);
        Mat3 result(300.0f, 360.0f, 420.0f, 660.0f, 810.0f, 960.0f, 1020.0f, 1260.0f, 1500.0f);

        assert(are_equal(m1 * m2, result));

        assert(are_equal(m1 * m2.col(0), result.col(0)));
        assert(are_equal(m1 * m2.col(1), result.col(1)));
        assert(are_equal(m1 * m2.col(2), result.col(2)));
    }

    // Rotation matrices
    {
        Vec3 x(1.0f, 0.0f, 0.0f);
        Vec3 y(0.0f, 1.0f, 0.0f);
        Vec3 z(0.0f, 0.0f, 1.0f);

        Mat3 Rx = Mat3::RotateX(3.14159f * 0.5f);
        Mat3 Ry = Mat3::RotateY(3.14159f * 0.5f);
        Mat3 Rz = Mat3::RotateZ(3.14159f * 0.5f);

        assert(are_equal(Rx*y, z));
        assert(are_equal(Rx*z, -y));

        assert(are_equal(Ry*z, x));
        assert(are_equal(Ry*x, -z));

        assert(are_equal(Rz*x, y));
        assert(are_equal(Rz*y, -x));
    }

    cout << "All tests passed." << endl;

    return 0;
}
