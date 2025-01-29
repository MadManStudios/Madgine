#pragma once

#include "renderdata.h"

#include "Madgine/imageloader/imageloader.h"

#include "Meta/math/atlas2.h"

#include "Meta/math/color4.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT ScalableImageRenderData : RenderData {

        void setImageName(std::string_view name, WidgetManager *mgr);
        void setImage(Resources::ImageLoader::Resource *image);

        std::string_view imageName() const;
        Resources::ImageLoader::Resource *image() const;

        void renderImage(WidgetsRenderData &renderData, Vector3 pos, Vector2 size, const Atlas2::Entry &entry, Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f });

        uint16_t mLeftBorder = 0;
        uint16_t mTopBorder = 0;
        uint16_t mBottomBorder = 0;
        uint16_t mRightBorder = 0;

    private:
        Resources::ImageLoader::Resource *mImage = nullptr;
    };

}
}