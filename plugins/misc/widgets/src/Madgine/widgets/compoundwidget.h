#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {

    struct WidgetTemplate {
    };

    struct MADGINE_WIDGETS_EXPORT CompoundWidget : Widget<CompoundWidget> {

        CompoundWidget(WidgetManager &mgr, WidgetTemplate _template, WidgetBase *parent = nullptr);

        void render(WidgetsRenderData &renderData) override;
        void updateChildrenGeometry() override;

        const std::vector<std::unique_ptr<WidgetBase>> &templateWidgets() const;

        WidgetBase *getTemplateWidget(std::string_view name);
        template <typename T>
        T *getTemplateWidget(std::string_view name)
        {
            return dynamic_cast<T *>(getTemplateWidget(name));
        }

    private:
        std::vector<std::unique_ptr<WidgetBase>> mTemplateWidgets;
    };

}
}