#include "../widgetslib.h"

#include "compoundwidget.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "label.h"
#include "image.h"
#include "widgetloader.h"
#include "widgetmanager.h"

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::CompoundWidget)

METATABLE_BEGIN_BASE(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
READONLY_PROPERTY(TemplateWidgets, templateWidgets)
METATABLE_END(Engine::Widgets::CompoundWidget)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetTemplate)
SERIALIZETABLE_END(Engine::Widgets::WidgetTemplate)

namespace Engine {
namespace Widgets {

    CompoundWidget::CompoundWidget(WidgetManager &mgr, WidgetTemplate _template, WidgetBase *parent)
        : Widget(mgr, parent)
    {
        std::unique_ptr<Image> image = std::make_unique<Image>(mgr, this);        

        image->mImageRenderData.setImageName("Explosion", &manager());
        image->mName = "Background";

        Label *label = image->createChild<Label>();
        label->mName = "Damage";
        label->mTextRenderData.setFontName("OpenSans-Bold");
        label->mText = "-15";
        label->mTextRenderData.mFontSize = 12;
        label->mTextRenderData.mColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        label->setSizeValue(7, 0.02f);
        label->setSizeValue(8, 0);
        mTemplateWidgets.push_back(std::move(image));
    }

    void CompoundWidget::render(WidgetsRenderData &renderData)
    {
        float oldAlpha = renderData.alpha();
        for (const std::unique_ptr<WidgetBase> &templateWidget : mTemplateWidgets) {
            renderData.setAlpha(oldAlpha * templateWidget->opacity());
            templateWidget->render(renderData);
        }
        renderData.setAlpha(oldAlpha);

        WidgetBase::render(renderData);
    }

    void CompoundWidget::updateChildrenGeometry()
    {
        WidgetBase::updateChildrenGeometry();

        for (const std::unique_ptr<WidgetBase> &child : mTemplateWidgets) {
            child->applyGeometry(getAbsoluteSize(), getAbsolutePosition());
        }
    }

    const std::vector<std::unique_ptr<WidgetBase>> &CompoundWidget::templateWidgets() const
    {
        return mTemplateWidgets;
    }

    WidgetBase *CompoundWidget::getTemplateWidget(std::string_view name)
    {
        for (const std::unique_ptr<WidgetBase>& templateWidget : mTemplateWidgets) {
            if (WidgetBase *widget = templateWidget->getChildRecursive(name))
                return widget;
        }
        return nullptr;
    }

}
}