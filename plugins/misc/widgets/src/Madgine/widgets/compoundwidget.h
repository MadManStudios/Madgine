#pragma once

#include "widget.h"
#include "widgetloader.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT CompoundWidget : WidgetBase {

        CompoundWidget(WidgetManager &mgr, WidgetLoader::Handle desc, WidgetBase *parent = nullptr);

        const char *getClass() const override;

        void render(WidgetsRenderData &renderData) override;
        void updateChildrenGeometry() override;

        const std::vector<std::unique_ptr<WidgetBase>> &templateWidgets() const;

        WidgetBase *getTemplateWidget(std::string_view name);
        const WidgetBase *getTemplateWidget(std::string_view name) const;
        template <typename T>
        T *getTemplateWidget(std::string_view name)
        {
            return dynamic_cast<T *>(getTemplateWidget(name));
        }

        virtual ScopePtr customScopePtr() override
        {
            return { this, mDescriptor->metaTable() };
        }

        WidgetBase *getChildRecursive(std::string_view name) override;

        WidgetBase *getHoveredDown(const Vector2 &point, const Rect2i &screenSpace) override;

    private:
        std::vector<std::unique_ptr<WidgetBase>> mTemplateWidgets;
        WidgetLoader::Handle mDescriptor;
    };

}
}