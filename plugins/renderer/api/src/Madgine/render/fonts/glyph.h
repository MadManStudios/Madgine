#pragma once

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Render {

    struct Glyph {
        Vector2i mSize;
        Vector2i mSize2;
        Vector2i mUV;
        Vector2i mUV2;
        Vector2i mBearing;
        int mAdvance;
        bool mFlipped;
        bool mFlipped2;
    };

}
}