#pragma once

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Render {

    struct Glyph {
        Math::Vector2i mSize;
        Math::Vector2i mSize2;
        Math::Vector2i mUV;
        Math::Vector2i mUV2;
        Math::Vector2i mBearing;
        int mAdvance;
        bool mFlipped;
        bool mFlipped2;
    };

}
}