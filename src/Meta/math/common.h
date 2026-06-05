#pragma once

namespace Engine {
namespace Math {

    const constexpr float floatZeroThreshold = 1e-06f;

    constexpr float abs(float f)
    {
        return f < 0.0f ? -f : f;
    }

    constexpr bool isZero(float f, float epsilon = floatZeroThreshold)
    {
        return abs(f) < epsilon;
    }

    constexpr bool isEqual(float a, float b, float epsilon = floatZeroThreshold)
    {
        return isZero(a - b, epsilon);
    }

    constexpr float sqrtNewtonRaphson(float x, float curr, float prev)
    {
        return curr == prev
            ? curr
            : sqrtNewtonRaphson(x, 0.5f * (curr + x / curr), curr);
    }

    constexpr float sqrtf(float x)
    {
        return x >= 0 && x < std::numeric_limits<float>::infinity()
            ? sqrtNewtonRaphson(x, x, 0)
            : std::numeric_limits<float>::quiet_NaN();
    }

    template <typename T>
    constexpr T min(const T &a, const T &b)
    {
        return a < b ? a : b;
    }

    template <typename T>
    constexpr T max(const T &a, const T &b)
    {
        return a > b ? a : b;
    }

    template <typename T>
    constexpr T clamp(const T &v, const T &minV, const T &maxV)
    {
        return min(max(v, minV), maxV);
    }

    constexpr int sign(int i)
    {
        return i == 0 ? 0 : (i < 0 ? -1 : 1);
    }

    constexpr float lerp(float a, float b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

}
}