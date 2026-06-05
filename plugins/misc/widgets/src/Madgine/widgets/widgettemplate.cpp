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

        Reflect::__Reflect_impl__::registerType(mMetaTable);
    }

    WidgetTemplate::~WidgetTemplate()
    {
        Reflect::__Reflect_impl__::unregisterType(mMetaTable);
    }

    std::unique_ptr<Reflect::Accessor[]> WidgetTemplate::accessors(const std::vector<WidgetData> &widgets)
    {
        std::unique_ptr<Reflect::Accessor[]> accessors = std::make_unique<Reflect::Accessor[]>(widgets.size() + 1);

        for (size_t i = 0; i < widgets.size(); i++) {
            accessors[i] = {
                widgets[i].mName.c_str(),
                nullptr,
                [](const Reflect::Accessor *self, Reflect::Value &out, const Reflect::Value &scope) -> Reflect::Result {
                    return invoke(out, [self](CompoundWidget &widget) { return widget.getTemplateWidget(self->mName); }, scope);
                },
                nullptr,
                { { Reflect::TypeEnum::ScopeValue }, WidgetLoader::load(widgets[i].mType)->metaTable()->mSelf }
            };
        }

        accessors[widgets.size()] = { nullptr };

        return accessors;
    }

}
}