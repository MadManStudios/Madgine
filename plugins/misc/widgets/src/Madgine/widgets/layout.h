#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Layout : Widget<Layout> {
        using Widget::Widget;
        virtual ~Layout() = default;

        void updateChildrenGeometry() override;
    };
}
}
