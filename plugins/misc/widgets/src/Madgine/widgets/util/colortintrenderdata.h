#pragma once

#include "renderdata.h"

#include "Meta/math/color4.h"

namespace Engine {
namespace Widgets {

    struct ColorTintRenderData : RenderData {

        Color4 mNormalColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 mHighlightedColor = { 1.2f, 1.2f, 1.2f, 1.0f };
        Color4 mPressedColor = { 1.3f, 1.3f, 1.3f, 1.0f };
        Color4 mSelectedColor = { 1.1f, 1.1f, 1.1f, 1.0f };
        Color4 mDisabledColor = { 0.4f, 0.4f, 0.4f, 1.0f };

    };

}
}