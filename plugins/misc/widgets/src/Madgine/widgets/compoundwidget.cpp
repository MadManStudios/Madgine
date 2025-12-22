#include "../widgetslib.h"

#include "compoundwidget.h"

#include "Meta/serialize/formats.h"

#include "Madgine/serialize/memory/memorymanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "image.h"
#include "label.h"
#include "widgetloader.h"
#include "widgetmanager.h"

METATABLE_BEGIN_BASE(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
    READONLY_PROPERTY(TemplateWidgets, templateWidgets)
METATABLE_END(Engine::Widgets::CompoundWidget)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::CompoundWidget)

namespace Engine {
namespace Widgets {

    CompoundWidget::CompoundWidget(WidgetManager &mgr, WidgetLoader::Handle desc, WidgetBase *parent)
        : WidgetBase(mgr, parent)
        , mDescriptor(std::move(desc))
    {
        // TODO move to resource thread
        Serialize::SerializeManager serializeMgr { "CompoundWidget" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), serializeMgr.wrapStream(mDescriptor.resource()->readAsStream(), true) };

        Serialize::StreamResult result = Serialize::read({ stream, CallerHierarchyBasePtr { CallerHierarchy<WidgetBase *> { this } }.append(&manager()) }, *this, "Widget");
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(result);
            throw 0;
        }

        mTemplateWidgets = std::move(mChildren);

        // image->mImageRenderData.setImageName("Explosion", &manager());
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

    const WidgetBase *CompoundWidget::getTemplateWidget(std::string_view name) const
    {
        return const_cast<CompoundWidget *>(this)->getTemplateWidget(name);
    }

    WidgetBase *CompoundWidget::getChildRecursive(std::string_view name)
    {
        for (WidgetBase *w : mTemplateWidgets | std::views::transform(projectionUniquePtrToPtr)) {
            if (WidgetBase *child = w->getChildRecursive(name)) {
                return child;
            }
        }

        return WidgetBase::getChildRecursive(name);
    }

    WidgetBase *CompoundWidget::getHoveredDown(const Vector2 &point, const Rect2i &screenSpace)
    {
        WidgetBase *hovered = WidgetBase::getHoveredDown(point, screenSpace);

        if (hovered == this) {
            for (WidgetBase *w : mTemplateWidgets | std::views::transform(projectionUniquePtrToPtr)) {
                if (WidgetBase *hovered = w->getHoveredDown(point, screenSpace)) {
                    return hovered;
                }
            }
        }

        return hovered;
    }

    WidgetBase *CompoundWidget::getTemplateWidget(std::string_view name)
    {
        for (const std::unique_ptr<WidgetBase> &templateWidget : mTemplateWidgets) {
            if (WidgetBase *widget = templateWidget->getChildRecursive(name))
                return widget;
        }
        return nullptr;
    }

    const char *CompoundWidget::getClass() const
    {
        return mDescriptor.name().data();
    }

}
}