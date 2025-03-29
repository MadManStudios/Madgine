#pragma once

#include "vector2.h"
#include "ray2.h"

namespace Engine {

struct Line2 {
    Vector2 mPointA;
    Vector2 mPointB;


    Ray2 toRay() const {
        return { mPointA, mPointB - mPointA };
    }

    float length() const {
        return (mPointB - mPointA).length();
    }

    Vector2 point(float ratio) const {
        return mPointA + (mPointB - mPointA) * ratio;
    }
};

}