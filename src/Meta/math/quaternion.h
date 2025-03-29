#pragma once

#include "matrix3.h"
#include "pi.h"
#include "vector3.h"

namespace Engine {

struct Quaternion {

    Quaternion()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
        , w(1.0f)
    {
    }

    Quaternion(float radian, const Vector3 &axis)
        : w(cosf(0.5f * radian))
    {
        Vector3 v = axis;
        v.normalize();
        v *= sinf(0.5f * radian);
        x = v.x;
        y = v.y;
        z = v.z;
    }

    Quaternion(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
        normalize();
    }

    constexpr explicit Quaternion(float *const r)
        : x(r[0])
        , y(r[1])
        , z(r[2])
        , w(r[3])
    {
    }

    static Quaternion FromRadian(const Vector3 &angles)
    {
        return Quaternion(angles.z, Vector3::UNIT_Z) * Quaternion(angles.x, Vector3::UNIT_X) * Quaternion(angles.y, Vector3::UNIT_Y);
    }

    static Quaternion FromDegrees(const Vector3 &angles)
    {
        return FromRadian(angles / 180.0f * PI);
    }

    Vector3 toRadian() const
    {
        return {
            std::atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y)),
            std::asin(2 * (w * y - z * x)),
            std::atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z))
        };
    }

    Vector3 toDegrees() const
    {
        return toRadian() / PI * 180.0f;
    }

    static Quaternion FromDirection(const NormalizedVector3 &dir, const NormalizedVector3 &up = { Vector3::UNIT_Y })
    {
        NormalizedVector3 side = up.crossProduct(dir);
        NormalizedVector3 actualUp = dir.crossProduct(side);

        Matrix3 m;
        m.FromAxes(side, actualUp, dir);
        return FromMatrix(m);
    }

    static Quaternion FromMatrix(const Matrix3 &m)
    {
        float qw, qx, qy, qz;
        float tr = m[0][0] + m[1][1] + m[2][2];

        if (tr > 0) {
            float S = sqrt(tr + 1.0) * 2; // S=4*qw
            qw = 0.25 * S;
            qx = (m[2][1] - m[1][2]) / S;
            qy = (m[0][2] - m[2][0]) / S;
            qz = (m[1][0] - m[0][1]) / S;
        } else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) {
            float S = sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2; // S=4*qx
            qw = (m[2][1] - m[1][2]) / S;
            qx = 0.25 * S;
            qy = (m[0][1] + m[1][0]) / S;
            qz = (m[0][2] + m[2][0]) / S;
        } else if (m[1][1] > m[2][2]) {
            float S = sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2; // S=4*qy
            qw = (m[0][2] - m[2][0]) / S;
            qx = (m[0][1] + m[1][0]) / S;
            qy = 0.25 * S;
            qz = (m[1][2] + m[2][1]) / S;
        } else {
            float S = sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2; // S=4*qz
            qw = (m[1][0] - m[0][1]) / S;
            qx = (m[0][2] + m[2][0]) / S;
            qy = (m[1][2] + m[2][1]) / S;
            qz = 0.25 * S;
        }

        assert(!isnan(qx) && !isnan(qy) && !isnan(qz) && !isnan(qw));
        return { qx, qy, qz, qw };
    }

    void operator*=(const Quaternion &other)
    {
        Vector3 v { x, y, z };
        Vector3 otherV { other.x, other.y, other.z };
        float newW = w * other.w - v.dotProduct(otherV);
        v = w * otherV + other.w * v + v.crossProduct(otherV);
        x = v.x;
        y = v.y;
        z = v.z;
        w = newW;
    }

    Quaternion operator*(const Quaternion &other) const
    {
        Quaternion q { *this };
        q *= other;
        return q;
    }

    void operator+=(const Quaternion &other)
    {
        w += other.w;
        x += other.x;
        y += other.y;
        z += other.z;
    }

    Quaternion operator+(const Quaternion &other)
    {
        Quaternion q { *this };
        q += other;
        return q;
    }

    void operator-=(const Quaternion &other)
    {
        w -= other.w;
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    Quaternion operator-(const Quaternion &other)
    {
        Quaternion q { *this };
        q -= other;
        return q;
    }

    void operator*=(float c)
    {
        x *= c;
        y *= c;
        z *= c;
        w *= c;
    }

    Quaternion operator*(float c)
    {
        Quaternion q { *this };
        q *= c;
        return q;
    }

    float dotProduct(const Quaternion &other)
    {
        return w * other.w + x * other.x + y * other.y + z * other.z;
    }

    void invert()
    {
        x *= -1;
        y *= -1;
        z *= -1;
    }

    Quaternion inverse() const
    {
        Quaternion q { *this };
        q.invert();
        return q;
    }

    Matrix3 toMatrix() const
    {
        return Matrix3 {
            1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w),
            2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w),
            2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)
        };
    }

    Vector3 operator*(const Vector3 &dir) const
    {
        return toMatrix() * dir;
    }

    bool operator==(const Quaternion &other) const
    {
        return w == other.w && x == other.x && y == other.y && z == other.z;
    }

    Quaternion operator-() const
    {
        return inverse();
    }

    void normalize()
    {
        float invNorm = 1.0f / sqrtf(w * w + x * x + y * y + z * z);
        x *= invNorm;
        y *= invNorm;
        z *= invNorm;
        w *= invNorm;
    }

    float x;
    float y;
    float z;
    float w;

    friend std::ostream &operator<<(std::ostream &o, const Quaternion &q)
    {
        o << "{" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "}";
        return o;
    }

    friend std::istream &operator>>(std::istream &in, Quaternion &q)
    {
        float *v = &q.x;
        char c;
        in >> c;
        if (c != '{')
            std::terminate();
        for (int i = 0; i < 4; ++i) {
            in >> v[i];
            in >> c;
            if (i != 3) {
                if (c != ',')
                    std::terminate();
            } else {
                if (c != '}')
                    std::terminate();
            }
        }
        return in;
    }

};

META_EXPORT Quaternion slerp(Quaternion q1, Quaternion q2, float ratio);
}