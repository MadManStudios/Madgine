#pragma once

#include "widget.h"


namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Layout : Widget<Layout> {
        using Widget::Widget;
        virtual ~Layout() = default;

        WidgetClass getClass() const override;

        void updateChildrenGeometry() override;
    };
}
}

REGISTER_TYPE(Engine::Widgets::Layout)
