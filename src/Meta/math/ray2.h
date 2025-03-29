#pragma once

#include "vector2.h"

namespace Engine {

struct Ray2 {
    Vector2 mPoint = Vector2::ZERO;
    NormalizedVector2 mDir = Vector2 { Vector2::ZERO };

    constexpr Vector2 point(float param) const
    {
        return mPoint + param * mDir;
    }
};
}