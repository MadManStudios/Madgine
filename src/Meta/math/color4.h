#pragma once

#include "color3.h"
#include "vector4.h"

namespace Engine {
namespace Math {

    struct Color4 {

        Color4() { }

        constexpr Color4(float r, float g, float b, float a)
            : r(r)
            , g(g)
            , b(b)
            , a(a)
        {
        }

        constexpr Color4(const Color3 &c, float a)
            : r(c.r)
            , g(c.g)
            , b(c.b)
            , a(a)
        {
        }

        constexpr Color4(float *const r)
            : r(r[0])
            , g(r[1])
            , b(r[2])
            , a(r[3])
        {
        }

        static constexpr Color4 fromHSV(float h, float s, float v, float a = 1.0f)
        {
            if (s == 0.0f) {
                // gray
                return { v, v, v, a };
            }

            h = fmodf(h, 1.0f) / (60.0f / 360.0f);
            int i = (int)h;
            float f = h - (float)i;
            float p = v * (1.0f - s);
            float q = v * (1.0f - s * f);
            float t = v * (1.0f - s * (1.0f - f));

            float r;
            float g;
            float b;
            switch (i) {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
            return { r, g, b, a };
        }

        constexpr Color4 operator+(const Color4 &rkVector) const
        {
            return Color4(
                r + rkVector.r,
                g + rkVector.g,
                b + rkVector.b,
                a + rkVector.a);
        }

        Color4 operator-(const Color4 &rkVector) const
        {
            return Color4(
                r - rkVector.r,
                g - rkVector.g,
                b - rkVector.b,
                a - rkVector.a);
        }

        Color4 operator*(const float fScalar) const
        {
            return Color4(
                r * fScalar,
                g * fScalar,
                b * fScalar,
                a * fScalar);
        }

        Color4 operator*(const Color4 &rhs) const
        {
            return Color4(
                r * rhs.r,
                g * rhs.g,
                b * rhs.b,
                a * rhs.a);
        }

        Color4 operator/(const float fScalar) const
        {
            assert(fScalar != 0.0);

            float fInv = 1.0f / fScalar;

            return Color4(
                r * fInv,
                g * fInv,
                b * fInv,
                a * fInv);
        }

        Color4 operator/(const Color4 &rhs) const
        {
            return Color4(
                r / rhs.r,
                g / rhs.g,
                b / rhs.b,
                a / rhs.a);
        }

        friend constexpr Color4 operator*(const float fScalar, const Color4 &rkVector)
        {
            return Color4(
                fScalar * rkVector.r,
                fScalar * rkVector.g,
                fScalar * rkVector.b,
                fScalar * rkVector.a);
        }

        friend Color4 operator/(const float fScalar, const Color4 &rkVector)
        {
            return Color4(
                fScalar / rkVector.r,
                fScalar / rkVector.g,
                fScalar / rkVector.b,
                fScalar / rkVector.a);
        }

        friend Color4 operator+(const Color4 &lhs, const float rhs)
        {
            return Color4(
                lhs.r + rhs,
                lhs.g + rhs,
                lhs.b + rhs,
                lhs.a + rhs);
        }

        friend Color4 operator+(const float lhs, const Color4 &rhs)
        {
            return Color4(
                lhs + rhs.r,
                lhs + rhs.g,
                lhs + rhs.b,
                lhs + rhs.a);
        }

        friend Color4 operator-(const Color4 &lhs, const float rhs)
        {
            return Color4(
                lhs.r - rhs,
                lhs.g - rhs,
                lhs.b - rhs,
                lhs.a - rhs);
        }

        friend Color4 operator-(const float lhs, const Color4 &rhs)
        {
            return Color4(
                lhs - rhs.r,
                lhs - rhs.g,
                lhs - rhs.b,
                lhs - rhs.a);
        }

        explicit constexpr operator Vector4() const
        {
            const auto toLinear = [](float x) {
                if (x <= 0.04045f) {
                    return x / 12.92f;
                } else {
                    return std::pow((x + 0.055f) / 1.055f, 2.4f);
                }
            };
            return { toLinear(r), toLinear(g), toLinear(b), a };
        }

        friend std::ostream &operator<<(std::ostream &o, const Color4 &c)
        {
            o << "(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
            return o;
        }

        friend std::istream &operator>>(std::istream &in, Color4 &color)
        {
            char c;
            in >> c;
            if (c != '(') {
                in.setstate(std::ios_base::failbit);
                return in;
            }
            for (int i = 0; i < 4; ++i) {
                in >> (&color.r)[i];
                in >> c;
                if (i != 3) {
                    if (c != ',') {
                        in.setstate(std::ios_base::failbit);
                        return in;
                    }
                } else {
                    if (c != ')') {
                        in.setstate(std::ios_base::failbit);
                        return in;
                    }
                }
            }
            return in;
        }

        float r;
        float g;
        float b;
        float a;
    };

    constexpr Color4 lerp(const Color4 &c1, const Color4 &c2, float ratio)
    {
        return (1.0f - ratio) * c1 + ratio * c2;
    }

}
}