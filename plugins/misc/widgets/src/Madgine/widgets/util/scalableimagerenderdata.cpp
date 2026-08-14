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
    MEMBER(mTopBorder)
    MEMBER(mBottomBorder)
    MEMBER(mRightBorder)
METATABLE_END(Engine::Widgets::ScalableImageRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::ScalableImageRenderData, Engine::Widgets::RenderData)
    ENCAPSULATED_FIELD(Image, imageName, setImageName, Engine::Serialize::Tags<"Image">)
    FIELD(mLeftBorder)
    FIELD(mTopBorder)
    FIELD(mBottomBorder)
    FIELD(mRightBorder)
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

        Math::Vector2 topLeftUV = Math::Vector2 { entry.mArea.mTopLeft + Math::Vector2i { 1, 1 } } / (2048.f /* * mData->mUIAtlasSize*/);
        Math::Vector2 uvSize = Math::Vector2 { entry.mArea.mSize - Math::Vector2i { 2, 2 } } / (2048.f /* * mData->mUIAtlasSize*/);
        Math::Vector2 bottomRightUV = topLeftUV + uvSize;

        Math::Vector2 topLeftUVOuter = topLeftUV;
        Math::Vector2 bottomRightUVOuter = bottomRightUV;

        const Math::Vector2i &imageSize = entry.mArea.mSize;

        if (mLeftBorder > 0) {
            pos.x += size.z * mLeftBorder;
            size.x -= size.z * mLeftBorder;
            topLeftUV.x += mLeftBorder * uvSize.x / imageSize.x;
        }

        if (mTopBorder > 0) {
            pos.y += size.z * mTopBorder;
            size.y -= size.z * mTopBorder;
            topLeftUV.y += mTopBorder * uvSize.y / imageSize.y;
        }

        if (mRightBorder > 0) {
            size.x -= size.z * mRightBorder;
            bottomRightUV.x -= mRightBorder * uvSize.x / imageSize.x;
        }

        if (mBottomBorder > 0) {
            size.y -= size.z * mBottomBorder;
            bottomRightUV.y -= mBottomBorder * uvSize.y / imageSize.y;
        }

        renderData.renderQuad(pos, size.xy(), color, {}, topLeftUV, bottomRightUV, entry.mFlipped);

        if (mLeftBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y }, { size.z * mLeftBorder, size.y }, color, {}, { topLeftUVOuter.x, topLeftUV.y }, { topLeftUV.x, bottomRightUV.y }, entry.mFlipped);

        if (mTopBorder > 0)
            renderData.renderQuad({ pos.x, posOuter.y }, { size.x, size.z * mTopBorder }, color, {}, { topLeftUV.x, topLeftUVOuter.y }, { bottomRightUV.x, topLeftUV.y }, entry.mFlipped);

        if (mRightBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y }, { size.z * mRightBorder, size.y }, color, {}, { bottomRightUV.x, topLeftUV.y }, { bottomRightUVOuter.x, bottomRightUV.y }, entry.mFlipped);

        if (mBottomBorder > 0)
            renderData.renderQuad({ pos.x, pos.y + size.y }, { size.x, size.z * mBottomBorder }, color, {}, { topLeftUV.x, bottomRightUV.y }, { bottomRightUV.x, bottomRightUVOuter.y }, entry.mFlipped);

        if (mLeftBorder > 0 && mTopBorder > 0)
            renderData.renderQuad(posOuter, { size.z * mLeftBorder, size.z * mTopBorder }, color, {}, topLeftUVOuter, topLeftUV, entry.mFlipped);

        if (mRightBorder > 0 && mBottomBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y + size.y }, { size.z * mRightBorder, size.z * mBottomBorder }, color, {}, bottomRightUV, bottomRightUVOuter, entry.mFlipped);

        if (mLeftBorder > 0 && mBottomBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y + size.y }, { size.z * mLeftBorder, size.z * mBottomBorder }, color, {}, { topLeftUVOuter.x, bottomRightUV.y }, { topLeftUV.x, bottomRightUVOuter.y }, entry.mFlipped);

        if (mRightBorder > 0 && mTopBorder > 0)
            renderData.renderQuad({ pos.x + size.x, posOuter.y }, { size.z * mRightBorder, size.z * mTopBorder }, color, {}, { bottomRightUV.x, topLeftUVOuter.y }, { bottomRightUVOuter.x, topLeftUV.y }, entry.mFlipped);
    }

}
}
