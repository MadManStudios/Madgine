#pragma once

#include "util/textrenderdata.h"
#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Label : Widget<Label> {
        using Widget::Widget;
        virtual ~Label() = default;

        void render(WidgetsRenderData &renderData) override;

        TextRenderData mTextRenderData;
        std::string mText;
    };
}
}
