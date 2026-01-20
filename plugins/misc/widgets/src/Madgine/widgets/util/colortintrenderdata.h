#pragma once

#include "Meta/math/color4.h"

#include "colorrenderdata.h"
#include "renderdata.h"

namespace Engine {
namespace Widgets {

    struct ColorTintRenderData : RenderData {

        ColorRenderData mNormalColor = Color4 { 1.0f, 1.0f, 1.0f, 1.0f };
        ColorRenderData mHighlightedColor = Color4 { 1.2f, 1.2f, 1.2f, 1.0f };
        ColorRenderData mPressedColor = Color4 { 1.3f, 1.3f, 1.3f, 1.0f };
        ColorRenderData mSelectedColor = Color4 { 1.1f, 1.1f, 1.1f, 1.0f };
        ColorRenderData mDisabledColor = Color4 { 0.4f, 0.4f, 0.4f, 1.0f };
    };

}
}