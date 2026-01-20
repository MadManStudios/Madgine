#pragma once

#include "util/scalableimagerenderdata.h"
#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Image : Widget<Image> {
        using Widget::Widget;
        virtual ~Image() = default;

        void render(WidgetsRenderData &renderData) override;

        ScalableImageRenderData mImageRenderData;
        ColorRenderData mColor;
    };
}
}
