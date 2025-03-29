#pragma once

#include "line2.h"
#include "vector2.h"

namespace Engine {

struct Rect2 {
    Vector2 mTopLeft, mSize;

    Rect2 &operator+=(const Vector2 &other)
    {
        mTopLeft += other;
        return *this;
    }

    float left() const
    {
        return mTopLeft.x;
    }

    float top() const
    {
        return mTopLeft.y;
    }

    float right() const
    {
        return mTopLeft.x + mSize.x;
    }

    float bottom() const
    {
        return mTopLeft.y + mSize.y;
    }

    Vector2 bottomRight() const
    {
        return mTopLeft + mSize;
    }

    Vector2 bottomLeft() const
    {
        return mTopLeft + Vector2 { 0.0f, mSize.y };
    }

    Vector2 topRight() const
    {
        return mTopLeft + Vector2 { mSize.x, 0.0f };
    }

    Line2 leftLine() const
    {
        return { mTopLeft, bottomLeft() };
    }

    Line2 topLine() const
    {
        return { mTopLeft, topRight() };
    }

    Line2 rightLine() const
    {
        return { topRight(), bottomRight() };
    }

    Line2 bottomLine() const
    {
        return { bottomLeft(), bottomRight() };
    }

    bool contains(Vector2 &point, bool exclusive = false) const
    {
        if (exclusive)
            return mTopLeft.x < point.x && point.x < mTopLeft.x + mSize.x
                && mTopLeft.y < point.y && point.y < mTopLeft.y + mSize.y;
        else
            return mTopLeft.x <= point.x && point.x <= mTopLeft.x + mSize.x
                && mTopLeft.y <= point.y && point.y <= mTopLeft.y + mSize.y;
    }
};

}