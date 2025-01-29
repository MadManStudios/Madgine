#include "../../widgetslib.h"

#include "scalableimagerenderdata.h"

#include "vertex.h"
#include "widgetsrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../widgetmanager.h"

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

    void ScalableImageRenderData::setImageName(std::string_view name, WidgetManager *mgr)
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

    void ScalableImageRenderData::renderImage(WidgetsRenderData &renderData, Vector3 pos, Vector2 size, const Atlas2::Entry &entry, Color4 color)
    {
        Vector3 posOuter = pos;

        Vector2 topLeftUV = Vector2 { entry.mArea.mTopLeft + Vector2i { 1, 1 } } / (2048.f /* * mData->mUIAtlasSize*/);
        Vector2 uvSize = Vector2 { entry.mArea.mSize - Vector2i { 2, 2 } } / (2048.f /* * mData->mUIAtlasSize*/);
        Vector2 bottomRightUV = topLeftUV + uvSize;

        Vector2 topLeftUVOuter = topLeftUV;
        Vector2 bottomRightUVOuter = bottomRightUV;

        const Vector2i &imageSize = entry.mArea.mSize;

        if (mLeftBorder > 0) {
            pos.x += mLeftBorder;
            size.x -= mLeftBorder;
            topLeftUV.x += mLeftBorder * uvSize.x / imageSize.x;
        }

        if (mTopBorder > 0) {
            pos.y += mTopBorder;
            size.y -= mTopBorder;
            topLeftUV.y += mTopBorder * uvSize.y / imageSize.y;
        }

        if (mRightBorder > 0) {
            size.x -= mRightBorder;
            bottomRightUV.x -= mRightBorder * uvSize.x / imageSize.x;
        }

        if (mBottomBorder > 0) {
            size.y -= mBottomBorder;
            bottomRightUV.y -= mBottomBorder * uvSize.y / imageSize.y;
        }

        renderData.renderQuad(pos, size, color, {}, topLeftUV, bottomRightUV);

        if (mLeftBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y, pos.z }, { static_cast<float>(mLeftBorder), size.y }, color, {}, { topLeftUVOuter.x, topLeftUV.y }, { topLeftUV.x, bottomRightUV.y });

        if (mTopBorder > 0)
            renderData.renderQuad({ pos.x, posOuter.y, pos.z }, { size.x, static_cast<float>(mTopBorder) }, color, {}, { topLeftUV.x, topLeftUVOuter.y }, { bottomRightUV.x, topLeftUV.y });

        if (mRightBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y, pos.z }, { static_cast<float>(mRightBorder), size.y }, color, {}, { bottomRightUV.x, topLeftUV.y }, { bottomRightUVOuter.x, bottomRightUV.y });

        if (mBottomBorder > 0)
            renderData.renderQuad({ pos.x, pos.y + size.y, pos.z }, { size.x, static_cast<float>(mBottomBorder) }, color, {}, { topLeftUV.x, bottomRightUV.y }, { bottomRightUV.x, bottomRightUVOuter.y });

        if (mLeftBorder > 0 && mTopBorder > 0)
            renderData.renderQuad(posOuter, { static_cast<float>(mLeftBorder), static_cast<float>(mTopBorder) }, color, {}, topLeftUVOuter, topLeftUV);

        if (mRightBorder > 0 && mBottomBorder > 0)
            renderData.renderQuad({ pos.x + size.x, pos.y + size.y, pos.z }, { static_cast<float>(mRightBorder), static_cast<float>(mBottomBorder) }, color, {}, bottomRightUV, bottomRightUVOuter);

        if (mLeftBorder > 0 && mBottomBorder > 0)
            renderData.renderQuad({ posOuter.x, pos.y + size.y, pos.z }, { static_cast<float>(mLeftBorder), static_cast<float>(mBottomBorder) }, color, {}, { topLeftUVOuter.x, bottomRightUV.y }, { topLeftUV.x, bottomRightUVOuter.y });

        if (mRightBorder > 0 && mTopBorder > 0)
            renderData.renderQuad({ pos.x + size.x, posOuter.y, pos.z }, { static_cast<float>(mRightBorder), static_cast<float>(mTopBorder) }, color, {}, { bottomRightUV.x, topLeftUVOuter.y }, { bottomRightUVOuter.x, topLeftUV.y });
    }

}
}
