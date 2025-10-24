#include "../widgetslib.h"

#include "widgettemplate.h"

#include "compoundwidget.h"

#include "Meta/keyvalue/accessor.h"

namespace Engine {
namespace Widgets {

	WidgetTemplate::WidgetTemplate(std::string name, std::vector<WidgetData> widgets)
        : mName(std::move(name))
        , mWidgets(std::move(widgets))
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