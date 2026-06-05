#pragma once

#include "vector3i.h"

namespace Engine {
namespace Math {

    struct Vector4i {
        int x, y, z, w;

        Vector4i() { }

        constexpr Vector4i(const int iX, const int iY, const int iZ, const int iW)
            : x(iX)
            , y(iY)
            , z(iZ)
            , w(iW)
        {
        }

        constexpr Vector4i(const int afCoordinate[4])
            : x(afCoordinate[0])
            , y(afCoordinate[1])
            , z(afCoordinate[2])
            , w(afCoordinate[3])
        {
        }

        explicit Vector4i(const Vector3i &xyz, const float w)
            : x(xyz.x)
            , y(xyz.y)
            , z(xyz.z)
            , w(w)
        {
        }

        constexpr Vector4i operator+(const Vector4i &other)
        {
            return { x + other.x, y + other.y, z + other.z, w + other.w };
        }

        constexpr Vector4i &operator+=(const Vector4i &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        constexpr bool operator==(const Vector4i &other) const
        {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        int operator[](const size_t i) const
        {
            assert(i < 4);

            return *(&x + i);
        }

        int &operator[](const size_t i)
        {
            assert(i < 4);

            return *(&x + i);
        }

        int *ptr()
        {
            return &x;
        }

        const int *ptr() const
        {
            return &x;
        }

        /** Swizzle-like narrowing operations
         */
        Vector2i xy() const
        {
            return { x, y };
        }

        static const constexpr int ZERO[] { 0, 0, 0, 0 };

        friend std::ostream &operator<<(std::ostream &o, const Vector4i &v)
        {
            o << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
            return o;
        }

        friend std::istream &operator>>(std::istream &in, Vector4i &v)
        {
            char c;
            in >> c;
            if (c != '[')
                std::terminate();
            for (int i = 0; i < 4; ++i) {
                in >> v[i];
                in >> c;
                if (i != 3) {
                    if (c != ',')
                        std::terminate();
                } else {
                    if (c != ']')
                        std::terminate();
                }
            }
            return in;
        }
    };

}
}