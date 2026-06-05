#pragma once

#include "vector3.h"

namespace Engine {
namespace Math {

    struct Color3 {

        Color3() { }

        constexpr Color3(float r, float g, float b)
            : r(r)
            , g(g)
            , b(b)
        {
        }

        constexpr Color3(const float r[3])
            : r(r[0])
            , g(r[1])
            , b(r[2])
        {
        }

        constexpr Color3(float *const r)
            : r(r[0])
            , g(r[1])
            , b(r[2])
        {
        }

        constexpr Color3 operator+(const Color3 &rkVector) const
        {
            return Color3(
                r + rkVector.r,
                g + rkVector.g,
                b + rkVector.b);
        }

        Color3 operator-(const Color3 &rkVector) const
        {
            return Color3(
                r - rkVector.r,
                g - rkVector.g,
                b - rkVector.b);
        }

        Color3 operator*(const float fScalar) const
        {
            return Color3(
                r * fScalar,
                g * fScalar,
                b * fScalar);
        }

        Color3 operator*(const Color3 &rhs) const
        {
            return Color3(
                r * rhs.r,
                g * rhs.g,
                b * rhs.b);
        }

        Color3 operator/(const float fScalar) const
        {
            assert(fScalar != 0.0);

            float fInv = 1.0f / fScalar;

            return Color3(
                r * fInv,
                g * fInv,
                b * fInv);
        }

        Color3 operator/(const Color3 &rhs) const
        {
            return Color3(
                r / rhs.r,
                g / rhs.g,
                b / rhs.b);
        }

        friend constexpr Color3 operator*(const float fScalar, const Color3 &rkVector)
        {
            return Color3(
                fScalar * rkVector.r,
                fScalar * rkVector.g,
                fScalar * rkVector.b);
        }

        friend Color3 operator/(const float fScalar, const Color3 &rkVector)
        {
            return Color3(
                fScalar / rkVector.r,
                fScalar / rkVector.g,
                fScalar / rkVector.b);
        }

        friend Color3 operator+(const Color3 &lhs, const float rhs)
        {
            return Color3(
                lhs.r + rhs,
                lhs.g + rhs,
                lhs.b + rhs);
        }

        friend Color3 operator+(const float lhs, const Color3 &rhs)
        {
            return Color3(
                lhs + rhs.r,
                lhs + rhs.g,
                lhs + rhs.b);
        }

        friend Color3 operator-(const Color3 &lhs, const float rhs)
        {
            return Color3(
                lhs.r - rhs,
                lhs.g - rhs,
                lhs.b - rhs);
        }

        friend Color3 operator-(const float lhs, const Color3 &rhs)
        {
            return Color3(
                lhs - rhs.r,
                lhs - rhs.g,
                lhs - rhs.b);
        }

        explicit constexpr operator Vector3() const
        {
            const auto toLinear = [](float x) {
                if (x <= 0.04045f) {
                    return x / 12.92f;
                } else {
                    return std::pow((x + 0.055f) / 1.055f, 2.4f);
                }
            };
            return { toLinear(r), toLinear(g), toLinear(b) };
        }

        friend std::ostream &operator<<(std::ostream &o, const Color3 &c)
        {
            o << "(" << c.r << ", " << c.g << ", " << c.b << ")";
            return o;
        }

        friend std::istream &operator>>(std::istream &in, Color3 &color)
        {
            char c;
            in >> c;
            if (c != '(')
                std::terminate();
            for (int i = 0; i < 3; ++i) {
                in >> (&color.r)[i];
                in >> c;
                if (i != 2) {
                    if (c != ',')
                        std::terminate();
                } else {
                    if (c != ')')
                        std::terminate();
                }
            }
            return in;
        }

        float r;
        float g;
        float b;
    };

    constexpr Color3 lerp(const Color3 &c1, const Color3 &c2, float ratio)
    {
        return (1.0f - ratio) * c1 + ratio * c2;
    }

}
}