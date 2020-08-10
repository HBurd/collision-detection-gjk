#include "math.hpp"

#include <cassert>
#include <cmath>

namespace demo::math
{

Vec3::Vec3(float x_, float y_, float z_)
    :x(x_), y(y_), z(z_)
{}

// Unary -
Vec3 Vec3::operator-() const
{
    return Vec3(-x, -y, -z);
}

// Unary +
Vec3 Vec3::operator+() const
{
    return *this;
}

Vec3 Vec3::operator+(const Vec3& rhs) const
{
    return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vec3 Vec3::operator-(const Vec3& rhs) const
{
    return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vec3 Vec3::operator+=(const Vec3& rhs)
{
    *this = *this + rhs;
    return *this;
}

Vec3 Vec3::operator-=(const Vec3& rhs)
{
    *this = *this - rhs;
    return *this;
}

float Vec3::sq_mag() const
{
    return x*x + y*y + z*z;
}

float Vec3::mag() const
{
    return sqrtf(sq_mag());
}

void Vec3::normalize()
{
    float magnitude = mag();
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}

Vec3 Vec3::X(float x)
{
    return Vec3(x, 0.0f, 0.0f);
}

Vec3 Vec3::Y(float y)
{
    return Vec3(0.0f, y, 0.0f);
}

Vec3 Vec3::Z(float z)
{
    return Vec3(0.0f, 0.0f, z);
}

Vec3 operator*(float lhs, const Vec3& rhs)
{
    return Vec3(lhs*rhs.x, lhs*rhs.y, lhs*rhs.z);
}

Vec3 operator*(const Vec3& lhs, float rhs)
{
    return rhs * lhs;
}

float dot(const Vec3& v1, const Vec3& v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec3 cross(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.y*v2.z - v1.z*v2.y,
                v1.z*v2.x - v1.x*v2.z,
                v1.x*v2.y - v1.y*v2.x);
}

Mat3::Mat3()
{
    *this = Identity();
}

Mat3::Mat3(float m00, float m01, float m02,
     float m10, float m11, float m12,
     float m20, float m21, float m22)
    : m{m00, m01, m02, m10, m11, m12, m20, m21, m22}
{}

Mat3 Mat3::Identity()
{
    return Mat3(1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f);
}

Mat3 Mat3::Zero()
{
    return Mat3(0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f);
}

Mat3 Mat3::FromColumns(const Vec3& c0, const Vec3& c1, const Vec3& c2)
{
    Mat3 result;
    result.set_col(0, c0);
    result.set_col(1, c1);
    result.set_col(2, c2);
    return result;
}

Mat3 Mat3::RotateX(float angle)
{
    float sin_a = sinf(angle);
    float cos_a = cosf(angle);

    return Mat3(1.0f, 0.0f, 0.0f,
                0.0f, cos_a, -sin_a,
                0.0f, sin_a, cos_a);
}

Mat3 Mat3::RotateY(float angle)
{
    float sin_a = sinf(angle);
    float cos_a = cosf(angle);

    return Mat3(cos_a, 0.0f, sin_a,
                0.0f,  1.0f, 0.0f,
                -sin_a, 0.0f, cos_a);
}

Mat3 Mat3::RotateZ(float angle)
{
    float sin_a = sinf(angle);
    float cos_a = cosf(angle);

    return Mat3(cos_a, -sin_a, 0.0f,
                sin_a, cos_a,  0.0f,
                0.0f,  0.0f,   1.0f);
}

Mat3 Mat3::AxisAngle(Vec3 axis_angle)
{
    float angle = axis_angle.mag();
    if (angle != 0.0f)
    {
        Vec3 axis = axis_angle * (1.0f / angle);

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        float c = cosf(angle);
        float s = sinf(angle);
        float C = 1.0f - c;

        return Mat3(
            x*x*C + c,   x*y*C - z*s, x*z*C + y*s,
            y*x*C + z*s, y*y*C + c,   y*z*C - x*s,
            z*x*C - y*s, z*y*C + x*s, z*z*C + c);
    }

    // When the angle is zero, there is no rotation.
    return Mat3::Identity();
}

float* Mat3::operator[](std::size_t index)
{
    return m[index];
}

const float* Mat3::operator[](std::size_t index) const
{
    return m[index];
}

Vec3 Mat3::row(unsigned int n) const
{
    assert(n <= 2);
    return Vec3(m[n][0], m[n][1], m[n][2]);
}

Vec3 Mat3::col(unsigned int n) const
{
    assert(n <= 2);
    return Vec3(m[0][n], m[1][n], m[2][n]);
}

void Mat3::set_row(unsigned int n, const Vec3& r)
{
    assert(n <= 2);
    m[n][0] = r.x;
    m[n][1] = r.y;
    m[n][2] = r.z;
}

void Mat3::set_col(unsigned int n, const Vec3& c)
{
    assert(n <= 2);
    m[0][n] = c.x;
    m[1][n] = c.y;
    m[2][n] = c.z;
}

Vec3 Mat3::operator*(const Vec3& rhs) const
{
    return Vec3(dot(row(0), rhs), dot(row(1), rhs), dot(row(2), rhs));
}

Mat3 Mat3::operator*(const Mat3& rhs) const
{
    return Mat3::FromColumns((*this) * rhs.col(0), (*this) * rhs.col(1), (*this) * rhs.col(2));
}

}
