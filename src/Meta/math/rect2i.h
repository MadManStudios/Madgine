#pragma once

#include "vector2i.h"

namespace Engine {

struct Rect2i {
    Vector2i mTopLeft, mSize;

    Vector2i bottomRight() const {
        return mTopLeft + mSize;
    }
};

}