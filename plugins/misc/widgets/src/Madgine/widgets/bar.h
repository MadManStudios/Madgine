#pragma once

#include "Generic/execution/bindable.h"

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Bar : Widget<Bar> {
        using Widget::Widget;
        virtual ~Bar() = default;

        void render(WidgetsRenderData &renderData) override;

        Execution::Bindable<float> mRatio = 0.0f;
        Execution::Bindable<Color4> mColor;
    };
}
}
