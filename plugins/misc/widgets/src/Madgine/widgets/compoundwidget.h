#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {

    struct WidgetTemplate {
    };

    struct CompoundWidget : Widget<CompoundWidget> {

        CompoundWidget(WidgetManager &mgr, WidgetTemplate _template, WidgetBase *parent = nullptr);

        void vertices(WidgetsRenderData &renderData, size_t layer = 0) override;
        void updateChildrenGeometry() override;

    private:
        std::vector<std::unique_ptr<WidgetBase>> mTemplateWidgets;
    };

}
}