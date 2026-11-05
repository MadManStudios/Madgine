#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT ScrollArea : Widget<ScrollArea> {
        using Widget::Widget;
        virtual ~ScrollArea() = default;

        bool appliesLayout() const override;
        void updateChildrenGeometry() override;

        bool injectAxisEvent(const Platform::Input::AxisEvent &arg) override;

        float mScroll = 0.0f;
        float mMaxScroll = 0.0f;
    };
}
}
