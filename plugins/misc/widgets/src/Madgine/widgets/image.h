#pragma once

#include "widget.h"

#include "util/scalableimagerenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Image : Widget<Image> {
        using Widget::Widget;
        virtual ~Image() = default;

        void render(WidgetsRenderData &renderData) override;

        std::string getClass() const override;

        ScalableImageRenderData mImageRenderData;
        Color4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    };
}
}

REGISTER_TYPE(Engine::Widgets::Image)
