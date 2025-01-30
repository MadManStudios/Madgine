#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Bar : Widget<Bar> {
        using Widget::Widget;
        virtual ~Bar() = default;

        void vertices(WidgetsRenderData &renderData, size_t layer) override;

        virtual WidgetClass getClass() const override;

        float mRatio = 0.0f;
        Color4 mColor;
    };
}
}
