#include "../../widgetslib.h"

#include "scalableimagerenderdata.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../widgetmanager.h"
#include "vertex.h"
#include "widgetsrenderdata.h"

METATABLE_BEGIN_BASE(Engine::Widgets::ScalableImageRenderData, Engine::Widgets::RenderData)
    PROPERTY(Image, image, setImage)
    MEMBER(mLeftBorder)
    MEMBER(mLeftBorderAbsolute)
    MEMBER(mTopBorder)
    MEMBER(mTopBorderAbsolute)
    MEMBER(mBottomBorder)
    MEMBER(mBottomBorderAbsolute)
    MEMBER(mRightBorder)
    MEMBER(mRightBorderAbsolute)
METATABLE_END(Engine::Widgets::ScalableImageRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::ScalableImageRenderData, Engine::Widgets::RenderData)
    ENCAPSULATED_FIELD(Image, imageName, setImageName, Engine::Serialize::Tags<"Image">)
    FIELD(mLeftBorder)
    FIELD(mLeftBorderAbsolute)
    FIELD(mTopBorder)
    FIELD(mTopBorderAbsolute)
    FIELD(mBottomBorder)
    FIELD(mBottomBorderAbsolute)
    FIELD(mRightBorder)
    FIELD(mRightBorderAbsolute)
SERIALIZETABLE_END(Engine::Widgets::ScalableImageRenderData)

namespace Engine {
namespace Widgets {

    void ScalableImageRenderData::setImageName(std::string_view name, Contextual<WidgetManager&> mgr)
    {
        setImage(mgr->getImage(name));
    }

    void ScalableImageRenderData::setImage(Resources::ImageLoader::Resource *image)
    {
        mImage = image;
    }

    std::string_view ScalableImageRenderData::imageName() const
    {
        return mImage ? mImage->name() : "";
    }

    Resources::ImageLoader::Resource *ScalableImageRenderData::image() const
    {
        return mImage;
    }

    void ScalableImageRenderData::renderImage(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, const ColorFrame &color)
    {
        Math::Vector2 posOuter = pos;

        Math::Vector2 topLeftUV = Math::Vector2 { entry.mArea.mTopLeft } / (2048.f /* * mData->mUIAtlasSize*/);
        Math::Vector2 uvSize = Math::Vector2 { entry.mArea.mSize } / (2048.f /* * mData->mUIAtlasSize*/);
        Math::Vector2 bottomRightUV = topLeftUV + uvSize;

        Math::Vector2 topLeftUVOuter = topLeftUV;
        Math::Vector2 bottomRightUVOuter = bottomRightUV;

        const Math::Vector2i &imageSize = entry.mArea.mSize;

        float leftBorder = size.z * mLeftBorder + mLeftBorderAbsolute;
        float topBorder = size.z * mTopBorder + mTopBorderAbsolute;
        float rightBorder = size.z * mRightBorder + mRightBorderAbsolute;
        float bottomBorder = size.z * mBottomBorder + mBottomBorderAbsolute;

        if (leftBorder > 0) {
            pos.x += leftBorder;
            size.x -= leftBorder;
            topLeftUV.x += (mLeftBorder + mLeftBorderAbsolute) * uvSize.x / imageSize.x;
        }

        if (topBorder > 0) {
            pos.y += topBorder;
            size.y -= topBorder;
            topLeftUV.y += (mTopBorder + mTopBorderAbsolute) * uvSize.y / imageSize.y;
        }

        if (rightBorder > 0) {
            size.x -= rightBorder;
            bottomRightUV.x -= (mRightBorder + mRightBorderAbsolute) * uvSize.x / imageSize.x;
        }

        if (bottomBorder > 0) {
            size.y -= bottomBorder;
            bottomRightUV.y -= (mBottomBorder + mBottomBorderAbsolute) * uvSize.y / imageSize.y;
        }

        renderData.renderQuad(pos, size.xy(), color, {}, topLeftUV, bottomRightUV, entry.mFlipped);

        if (leftBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y }, { leftBorder, size.y }, color, {}, { topLeftUVOuter.x, topLeftUV.y }, { topLeftUV.x, bottomRightUV.y }, entry.mFlipped);

        if (topBorder > 0)
            renderData.renderQuad({ pos.x, posOuter.y }, { size.x, topBorder }, color, {}, { topLeftUV.x, topLeftUVOuter.y }, { bottomRightUV.x, topLeftUV.y }, entry.mFlipped);

        if (rightBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y }, { rightBorder, size.y }, color, {}, { bottomRightUV.x, topLeftUV.y }, { bottomRightUVOuter.x, bottomRightUV.y }, entry.mFlipped);

        if (bottomBorder > 0)
            renderData.renderQuad({ pos.x, pos.y + size.y }, { size.x, bottomBorder }, color, {}, { topLeftUV.x, bottomRightUV.y }, { bottomRightUV.x, bottomRightUVOuter.y }, entry.mFlipped);

        if (leftBorder > 0 && topBorder > 0)
            renderData.renderQuad(posOuter, { leftBorder, topBorder }, color, {}, topLeftUVOuter, topLeftUV, entry.mFlipped);

        if (rightBorder > 0 && bottomBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y + size.y }, { rightBorder, bottomBorder }, color, {}, bottomRightUV, bottomRightUVOuter, entry.mFlipped);

        if (leftBorder > 0 && bottomBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y + size.y }, { leftBorder, bottomBorder }, color, {}, { topLeftUVOuter.x, bottomRightUV.y }, { topLeftUV.x, bottomRightUVOuter.y }, entry.mFlipped);

        if (rightBorder > 0 && topBorder > 0)
            renderData.renderQuad({ pos.x + size.x, posOuter.y }, { rightBorder, topBorder }, color, {}, { bottomRightUV.x, topLeftUVOuter.y }, { bottomRightUVOuter.x, topLeftUV.y }, entry.mFlipped);
    }

}
}
