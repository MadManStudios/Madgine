#include "../metalib.h"

#include "bounds.h"

#include "vector2.h"

namespace Engine {
Bounds::Bounds(float left, float bottom, float right, float top)
    : mLeft(left)
    , mBottom(bottom)
    , mRight(right)
    , mTop(top)

{
}

float Bounds::left() const
{
    return mLeft;
}

float Bounds::top() const
{
    return mTop;
}

float Bounds::right() const
{
    return mRight;
}

float Bounds::bottom() const
{
    return mBottom;
}

float Bounds::width() const
{
    return std::abs(mRight - mLeft);
}

float Bounds::height() const
{
    return std::abs(mTop - mBottom);
}

Vector2 Bounds::size() const
{
    return { width(), height() };
}

Vector2 Bounds::bottomLeft() const
{
    return { mLeft, mBottom };
}

Vector2 Bounds::topLeft() const
{
    return { mLeft, mTop };
}

Vector2 Bounds::bottomRight() const
{
    return { mRight, mBottom };
}

Vector2 Bounds::topRight() const
{
    return { mRight, mTop };
}

std::array<Vector2, 4> Bounds::corners() const
{
    return { { { mLeft, mBottom }, { mLeft, mTop }, { mRight, mTop }, { mRight, mBottom } } };
}

Vector2 Bounds::operator*(const Vector2 &v) const
{
    return { mLeft + width() * v.x, mBottom + height() * v.y };
}

bool Bounds::isNull() const
{
    return width() <= 0 || height() <= 0;
}
}
