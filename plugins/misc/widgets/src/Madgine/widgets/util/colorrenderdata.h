#pragma once

#pragma once

#include "Meta/math/color4.h"

#include "renderdata.h"

namespace Engine {
namespace Widgets {

    struct ColorQuad {

        Color4 mTopLeft = { 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 mTopRight = { 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 mBottomLeft = { 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 mBottomRight = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    struct ColorRenderData : RenderData {

        ColorRenderData() = default;
        ColorRenderData(Color4 color)
            : mColor(color)
        {
        }

        ColorFrame frame(Vector2 pos, Vector2 size) const;

        Color4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Color4 mSecondaryColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Vector2 mGradient = { 0.0f, 0.0f };
    };

    struct ColorFrame {
        ColorQuad toQuad(Vector2 pos, Vector2 size) const;

        ColorRenderData mRenderData;
        Vector2 mPos { 0.0f, 0.0f };
        Vector2 mSize { 1.0f, 1.0f };
    };

}
}