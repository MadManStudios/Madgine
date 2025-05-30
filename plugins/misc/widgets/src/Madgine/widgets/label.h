#pragma once

#include "widget.h"

#include "util/textrenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Label : Widget<Label> {
        using Widget::Widget;
        virtual ~Label() = default;

        std::string getClass() const override;

        void render(WidgetsRenderData &renderData) override;

        TextRenderData mTextRenderData;
        std::string mText;
    };
}
}

REGISTER_TYPE(Engine::Widgets::Label)
