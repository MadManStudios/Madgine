#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Checkbox : Widget<Checkbox> {
        using Widget::Widget;
        virtual ~Checkbox() = default;

        virtual bool isChecked();
        virtual void setChecked(bool b);

        virtual WidgetClass getClass() const override;
    };
}
}
