#pragma once

#include "widget.h"

#include "util/textrenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Label : Widget<Label> {
        using Widget::Widget;
        virtual ~Label() = default;

        virtual WidgetClass getClass() const override;

        void vertices(WidgetsRenderData &renderData, size_t layer) override;

        TextRenderData mTextRenderData;
        std::string mText;
    };
}
}

REGISTER_TYPE(Engine::Widgets::Label)
