#include "../widgetslib.h"

#include "widgettemplate.h"

#include "Meta/reflect/accessor.h"

#include "compoundwidget.h"

namespace Engine {
namespace Widgets {

    WidgetTemplate::WidgetTemplate(std::string name, std::vector<WidgetData> widgets)
        : mName(std::move(name))
        , mWidgets(std::move(widgets))
        , mAccessors(accessors(mWidgets))
        , mMetaTable(&mSelfTable, mName.c_str(), mAccessors.get())
    {
        mMetaTable.mBase = &table<CompoundWidget>;

        Type::registerMetaTable(mMetaTable);
    }

    WidgetTemplate::~WidgetTemplate()
    {
        Type::unregisterMetaTable(mMetaTable);
    }

    std::unique_ptr<Reflect::Accessor[]> WidgetTemplate::accessors(const std::vector<WidgetData> &widgets)
    {
        std::unique_ptr<Reflect::Accessor[]> accessors = std::make_unique<Reflect::Accessor[]>(widgets.size() + 1);

        for (size_t i = 0; i < widgets.size(); i++) {

            const Reflect::MetaTable **type;
            auto it = WidgetRegistry::sComponentsByName().find(widgets[i].mType);
            if (it == WidgetRegistry::sComponentsByName().end()) {
                type = WidgetLoader::load(widgets[i].mType)->metaTable()->mSelf;
            } else {
                type = WidgetRegistry::get(it->second).Reflect::TypeAnnotation::mType;
            }

            accessors[i] = {
                widgets[i].mName.c_str(),
                nullptr,
                [](const Reflect::Accessor *self, Reflect::Value &out, const Reflect::Value &scope, Reflect::ContextPtr context) -> Reflect::Result {
                    return invoke_member(out, [self](CompoundWidget &widget) { return widget.getTemplateWidget(self->mName); }, context, scope);
                },
                nullptr,
                { { Reflect::TypeEnum::ScopeValue }, type }
            };
        }

        accessors[widgets.size()] = { nullptr };

        return accessors;
    }

}
}