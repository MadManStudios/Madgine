#pragma once

#include "widget.h"

#include "util/scalableimagerenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Image : Widget<Image> {
        using Widget::Widget;
        virtual ~Image() = default;

        void vertices(WidgetsRenderData &renderData, size_t layer) override;

        virtual std::string getClass() const override;

        ScalableImageRenderData mImageRenderData;
    };
}
}

REGISTER_TYPE(Engine::Widgets::Image)
