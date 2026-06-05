#pragma once

#include "Madgine/behavior/bindable.h"

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Bar : Widget<Bar> {
        using Widget::Widget;
        virtual ~Bar() = default;

        void render(WidgetsRenderData &renderData) override;

        Behavior::Bindable<float> mRatio = 0.0f;
        Behavior::Bindable<Math::Color4> mColor;
    };
}
}
