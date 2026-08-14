#pragma once

#include "Generic/context.h"

#include "Meta/math/atlas2.h"
#include "Meta/math/color4.h"

#include "Madgine/imageloader/imageloader.h"

#include "colorrenderdata.h"
#include "renderdata.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT ScalableImageRenderData : RenderData {

        void setImageName(std::string_view name, Contextual<WidgetManager &> mgr);
        void setImage(Resources::ImageLoader::Resource *image);

        std::string_view imageName() const;
        Resources::ImageLoader::Resource *image() const;

        void renderImage(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, const ColorFrame &color = {});

        uint16_t mLeftBorder = 0;
        uint16_t mTopBorder = 0;
        uint16_t mBottomBorder = 0;
        uint16_t mRightBorder = 0;

    private:
        Resources::ImageLoader::Resource *mImage = nullptr;
    };

}
}