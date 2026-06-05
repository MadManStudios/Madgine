#include "../../widgetslib.h"

#include "colorrenderdata.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::ColorRenderData, Engine::Widgets::RenderData)
    MEMBER(mColor)
    MEMBER(mSecondaryColor)
    MEMBER(mGradient)
METATABLE_END(Engine::Widgets::ColorRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::ColorRenderData, Engine::Widgets::RenderData)
    FIELD(mColor)
    FIELD(mSecondaryColor)
    FIELD(mGradient)
SERIALIZETABLE_END(Engine::Widgets::ColorRenderData)

namespace Engine {
namespace Widgets {

    ColorFrame ColorRenderData::frame(Math::Vector2 pos, Math::Vector2 size) const
    {
        return { *this, pos, size };
    }

    ColorQuad ColorFrame::toQuad(Math::Vector2 pos, Math::Vector2 size) const
    {
        Math::Vector2 v = mPos;
        float min = v.dotProduct(mRenderData.mGradient);
        float max = min;        
        v.x += mSize.x;
        min = std::min(min, v.dotProduct(mRenderData.mGradient));
        max = std::max(max, v.dotProduct(mRenderData.mGradient));
        v.y += mSize.y;
        min = std::min(min, v.dotProduct(mRenderData.mGradient));
        max = std::max(max, v.dotProduct(mRenderData.mGradient));
        v.x -= mSize.x;
        min = std::min(min, v.dotProduct(mRenderData.mGradient));
        max = std::max(max, v.dotProduct(mRenderData.mGradient));

        Math::Color4 topLeft, topRight, bottomLeft, bottomRight;

        v = pos;
        float f = v.dotProduct(mRenderData.mGradient);
        topLeft = lerp(mRenderData.mColor, mRenderData.mSecondaryColor, Math::clamp((f - min) / (max - min), 0.0f, 1.0f));
        v.x += size.x;
        f = v.dotProduct(mRenderData.mGradient);
        topRight = lerp(mRenderData.mColor, mRenderData.mSecondaryColor, Math::clamp((f - min) / (max - min), 0.0f, 1.0f));
        v.y += size.y;
        f = v.dotProduct(mRenderData.mGradient);
        bottomRight = lerp(mRenderData.mColor, mRenderData.mSecondaryColor, Math::clamp((f - min) / (max - min), 0.0f, 1.0f));
        v.x -= size.x;
        f = v.dotProduct(mRenderData.mGradient);
        bottomLeft = lerp(mRenderData.mColor, mRenderData.mSecondaryColor, Math::clamp((f - min) / (max - min), 0.0f, 1.0f));

        return { topLeft, topRight, bottomLeft, bottomRight};
    }

}
}
