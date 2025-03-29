#pragma once

#include "vector3.h"

namespace Engine {

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

    constexpr operator Vector3() const
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

}