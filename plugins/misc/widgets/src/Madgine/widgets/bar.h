#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Bar : Widget<Bar> {
        using Widget::Widget;
        virtual ~Bar() = default;

        virtual void setRatio(float f);

        virtual WidgetClass getClass() const override;
    };
}
}
