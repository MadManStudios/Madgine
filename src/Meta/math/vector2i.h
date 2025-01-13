#pragma once

namespace Engine {

struct Vector2i {
    int x, y;

    Vector2i() = default;

    constexpr Vector2i(int x, int y)
        : x(x)
        , y(y)
    {
    }

    explicit Vector2i(int *const r)
        : x(r[0])
        , y(r[1])
    {
    }

    /*constexpr Vector2i(const InterfacesVector &v)
        : x(v.x)
        , y(v.y)
    {
    }

    constexpr operator InterfacesVector() const {
        return { x, y };
    }*/

    constexpr Vector2i(const Vector2i &) = default;

    constexpr Vector2i &operator+=(const Vector2i &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2i operator+(const Vector2i &other) const
    {
        return { x + other.x, y + other.y };
    }

    constexpr Vector2i operator-(const Vector2i &other) const
    {
        return { x - other.x, y - other.y };
    }

    constexpr bool operator==(const Vector2i &other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr int operator[](const size_t i) const
    {
        assert(i < 2);

        return *(&x + i);
    }

    constexpr int &operator[](const size_t i)
    {
        assert(i < 2);

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

    friend std::ostream &operator<<(std::ostream &o, const Vector2i &v)
    {
        o << "[" << v.x << ", " << v.y << "]";
        return o;
    }

    friend std::istream &operator>>(std::istream &in, Vector2i &v)
    {
        char c;
        in >> c;
        if (c != '[')
            std::terminate();
        for (int i = 0; i < 2; ++i) {
            in >> v[i];
            in >> c;
            char expectedChar = (i == 1) ? ']' : ',';
            if (c != expectedChar)
                std::terminate();
        }
        return in;
    }
};

constexpr Vector2i min(const Vector2i &first, const Vector2i &second)
{
    return {
        std::min(first.x, second.x), std::min(first.y, second.y)
    };
}

}
