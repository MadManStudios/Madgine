#include "../widgetslib.h"

#include "compoundwidget.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "label.h"

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::CompoundWidget)

METATABLE_BEGIN_BASE(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::CompoundWidget)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetTemplate)
SERIALIZETABLE_END(Engine::Widgets::WidgetTemplate)

namespace Engine {
namespace Widgets {

    CompoundWidget::CompoundWidget(WidgetManager &mgr, WidgetTemplate _template, WidgetBase *parent)
        : Widget(mgr, parent)
    {
        std::unique_ptr<Label> label = std::make_unique<Label>(mgr, this);
        label->mTextRenderData.setFontName("OpenSans-Regular");
        label->mText = "Hello World!";
        mTemplateWidgets.push_back(std::move(label));
    }

    void CompoundWidget::vertices(WidgetsRenderData &renderData, size_t layer)
    {
        for (const std::unique_ptr<WidgetBase> &templateWidget : mTemplateWidgets) {
            templateWidget->vertices(renderData, layer);
        }
    }

    void CompoundWidget::updateChildrenGeometry()
    {
        WidgetBase::updateChildrenGeometry();

        for (const std::unique_ptr<WidgetBase> &child : mTemplateWidgets)
        {
            child->applyGeometry(getAbsoluteSize(), getAbsolutePosition());
        }
    }

}
}