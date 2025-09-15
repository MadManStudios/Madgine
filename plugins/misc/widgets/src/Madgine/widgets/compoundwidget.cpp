#include "../widgetslib.h"

#include "compoundwidget.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "image.h"
#include "label.h"
#include "widgetloader.h"
#include "widgetmanager.h"

#include "Madgine/serialize/memory/memorymanager.h"

#include "Meta/serialize/formats.h"

METATABLE_BEGIN_BASE(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::CompoundWidget)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::CompoundWidget, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::CompoundWidget)

namespace Engine {
namespace Widgets {

    CompoundWidget::CompoundWidget(WidgetManager &mgr, WidgetLoader::Handle desc, WidgetBase *parent)
        : WidgetBase(mgr, parent)
        , mDescriptor(std::move(desc))
    {

        Serialize::SerializeManager serializeMgr { "CompoundWidget" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), serializeMgr.wrapStream(mDescriptor.resource()->readAsStream(), true) };

        Serialize::StreamResult result = Serialize::read<std::vector<std::unique_ptr<WidgetBase>>, Serialize::ParentCreator<&CompoundWidget::readWidget, &CompoundWidget::writeWidget, nullptr, &WidgetManager::scanWidget>>(stream, mTemplateWidgets, "Widgets", CallerHierarchy<WidgetBase*> { this });
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(result);
            throw 0;
        }

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

    WidgetBase *CompoundWidget::getTemplateWidget(std::string_view name)
    {
        for (const std::unique_ptr<WidgetBase> &templateWidget : mTemplateWidgets) {
            if (WidgetBase *widget = templateWidget->getChildRecursive(name))
                return widget;
        }
        return nullptr;
    }

    WidgetTemplate::WidgetTemplate(std::string name, std::vector<WidgetData> widgets)
        : mName(std::move(name))
        , mWidgets(std::move(widgets))
        , mSerializers(serializers(mWidgets))
        , mSerializeTable(mName.c_str(), type_holder<CompoundWidget>, &serializeTable<CompoundWidget>, Serialize::__serialize_impl__::readState<CompoundWidget>, mSerializers.get(), nullptr, false)
        , mAccessors(accessors(mWidgets))
        , mMetaTable(&mSelfTable, mName.c_str(), mAccessors.get())
    {
        mMetaTable.mBase = &table<CompoundWidget>;

        registerType(mMetaTable);
    }

    WidgetTemplate::~WidgetTemplate()
    {
        unregisterType(mMetaTable);
    }

    std::unique_ptr<Serialize::Serializer[]> WidgetTemplate::serializers(const std::vector<WidgetData> &widgets)
    {
        std::unique_ptr<Serialize::Serializer[]> serializers = std::make_unique<Serialize::Serializer[]>(widgets.size() + 1);

        for (size_t i = 0; i < widgets.size(); ++i) {
            serializers[i] = {
                widgets[i].mName.c_str(),
                nullptr,
                [](const void *obj, Serialize::FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) {
                    const CompoundWidget *compound = static_cast<const CompoundWidget *>(obj);
                    const WidgetBase *w = compound->getTemplateWidget(name);
                    Serialize::write(out, *w, name, hierarchy);
                },
                [](void *obj, Serialize::FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy) {
                    CompoundWidget *compound = static_cast<CompoundWidget *>(obj);
                    WidgetBase *w = compound->getTemplateWidget(name);
                    return Serialize::read(in, *w, name, hierarchy);
                }

            };
        }

        serializers[widgets.size()] = { nullptr };

        return serializers;
    }

    std::unique_ptr<Accessor[]> WidgetTemplate::accessors(const std::vector<WidgetData> &widgets)
    {
        std::unique_ptr<Accessor[]> accessors = std::make_unique<Accessor[]>(widgets.size() + 1);

        for (size_t i = 0; i < widgets.size(); i++) {
            accessors[i] = {
                widgets[i].mName.c_str(),
                nullptr,
                [](const Accessor *self, ValueType &out, const ScopePtr &scope) {
                    CompoundWidget *compound = scope_cast<CompoundWidget>(scope);
                    WidgetBase *w = compound->getTemplateWidget(self->mName);
                    to_ValueType(out, w);
                },
                nullptr,
                { { ValueTypeEnum::ScopeValue }, WidgetLoader::load(widgets[i].mType)->metaTable()->mSelf }
            };
        }

        accessors[widgets.size()] = { nullptr };

        return accessors;
    }

}
}