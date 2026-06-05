#pragma once

namespace Engine {
namespace Math {

    struct META_EXPORT Bounds {
        Bounds(float left = 0.f, float bottom = 0.f, float right = 0.f, float top = 0.f);

        float left() const;
        float top() const;
        float right() const;
        float bottom() const;

        float width() const;
        float height() const;

        Vector2 size() const;

        Vector2 bottomLeft() const;
        Vector2 topLeft() const;
        Vector2 bottomRight() const;
        Vector2 topRight() const;

        std::array<Vector2, 4> corners() const;

        Vector2 operator*(const Vector2 &v) const;

        bool isNull() const;

    private:
        float mLeft, mBottom, mRight, mTop;
    };

}
}
