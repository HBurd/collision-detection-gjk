#ifndef MATH_HPP
#define MATH_HPP

#include <cstddef>

namespace demo::math
{

struct Vec3
{
    using real_type = float;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_);

    // Unary +, -
    Vec3 operator-() const;
    Vec3 operator+() const;

    Vec3 operator+(const Vec3& rhs) const;
    Vec3 operator-(const Vec3& rhs) const;
    Vec3 operator+=(const Vec3& rhs);
    Vec3 operator-=(const Vec3& rhs);

    // square magnitude
    float sq_mag();

    // magnitude
    float mag();

    static Vec3 X(float x);
    static Vec3 Y(float y);
    static Vec3 Z(float z);
};

// Scalar multiples
Vec3 operator*(float lhs, const Vec3& rhs);
Vec3 operator*(const Vec3& lhs, float rhs);

float dot(const Vec3& v1, const Vec3& v2);

struct Mat3
{
    float m[3][3];

    Mat3();
    Mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);

    static Mat3 Identity();
    static Mat3 Zero();

    static Mat3 FromColumns(const Vec3& c0, const Vec3& c1, const Vec3& c2);

    static Mat3 RotateX(float angle);
    static Mat3 RotateY(float angle);
    static Mat3 RotateZ(float angle);

    float* operator[](std::size_t index);
    const float* operator[](std::size_t index) const;

    Vec3 row(unsigned int n) const;
    Vec3 col(unsigned int n) const;

    void set_row(unsigned int n, const Vec3& r);
    void set_col(unsigned int n, const Vec3& c);

    Vec3 operator*(const Vec3& rhs) const;
    Mat3 operator*(const Mat3& rhs) const;
};

}

#endif
