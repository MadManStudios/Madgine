#pragma once

#include "Meta/math/color4.h"

#include "colorrenderdata.h"
#include "renderdata.h"

namespace Engine {
namespace Widgets {

    struct ColorTintRenderData : RenderData {

        ColorRenderData mNormalColor = Math::Color4 { 1.0f, 1.0f, 1.0f, 1.0f };
        ColorRenderData mHighlightedColor = Math::Color4 { 1.2f, 1.2f, 1.2f, 1.0f };
        ColorRenderData mPressedColor = Math::Color4 { 1.3f, 1.3f, 1.3f, 1.0f };
        ColorRenderData mSelectedColor = Math::Color4 { 1.1f, 1.1f, 1.1f, 1.0f };
        ColorRenderData mDisabledColor = Math::Color4 { 0.4f, 0.4f, 0.4f, 1.0f };
    };

}
}