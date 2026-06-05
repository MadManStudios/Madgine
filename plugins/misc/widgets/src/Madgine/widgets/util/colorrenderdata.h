#pragma once

#pragma once

#include "Meta/math/color4.h"

#include "renderdata.h"

namespace Engine {
namespace Widgets {

    struct ColorQuad {

        Math::Color4 mTopLeft = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 mTopRight = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 mBottomLeft = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 mBottomRight = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    struct ColorRenderData : RenderData {

        ColorRenderData() = default;
        ColorRenderData(Math::Color4 color)
            : mColor(color)
        {
        }

        ColorFrame frame(Math::Vector2 pos, Math::Vector2 size) const;

        Math::Color4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 mSecondaryColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Vector2 mGradient = { 0.0f, 0.0f };
    };

    struct ColorFrame {
        ColorQuad toQuad(Math::Vector2 pos, Math::Vector2 size) const;

        ColorRenderData mRenderData;
        Math::Vector2 mPos { 0.0f, 0.0f };
        Math::Vector2 mSize { 1.0f, 1.0f };
    };

}
}