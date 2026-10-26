#pragma once

#include "vector2i.h"

namespace Engine {
namespace Math {

    struct Vector3i {
        int x = 0, y = 0, z = 0;

        constexpr Vector3i() = default;

        constexpr Vector3i(const int iX, const int iY, const int iZ)
            : x(iX)
            , y(iY)
            , z(iZ)
        {
        }

        constexpr Vector3i(const int afCoordinate[3])
            : x(afCoordinate[0])
            , y(afCoordinate[1])
            , z(afCoordinate[2])
        {
        }

        explicit constexpr Vector3i(const Vector2i &xy, const float z)
            : x(xy.x)
            , y(xy.y)
            , z(z)
        {
        }

        constexpr Vector3i operator+(const Vector3i &other)
        {
            return { x + other.x, y + other.y, z + other.z };
        }

        constexpr Vector3i &operator+=(const Vector3i &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        constexpr bool operator==(const Vector3i &other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        constexpr int operator[](const size_t i) const
        {
            assert(i < 3);

            return *(&x + i);
        }

        constexpr int &operator[](const size_t i)
        {
            assert(i < 3);

            return *(&x + i);
        }

        constexpr int *ptr()
        {
            return &x;
        }

        constexpr const int *ptr() const
        {
            return &x;
        }

        /** Swizzle-like narrowing operations
         */
        constexpr Vector2i xy() const
        {
            return { x, y };
        }

        static const constexpr int ZERO[] { 0, 0, 0 };

        friend std::ostream &operator<<(std::ostream &o, const Vector3i &v)
        {
            o << "[" << v.x << ", " << v.y << ", " << v.z << "]";
            return o;
        }

        friend std::istream &operator>>(std::istream &in, Vector3i &v)
        {
            char c;
            in >> c;
            if (c != '[')
                std::terminate();
            for (int i = 0; i < 3; ++i) {
                in >> v[i];
                in >> c;
                char expectedChar = (i == 2) ? ']' : ',';
                if (c != expectedChar)
                    std::terminate();
            }
            return in;
        }
    };

}
}