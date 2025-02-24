#pragma once

#include "Madgine/resources/resourceloader.h"
#include "compoundwidget.h"
#include "Meta/serialize/hierarchy/serializetable_forward.h"

namespace Engine {
namespace Widgets {

    struct WidgetDescriptor {
        WidgetDescriptor(std::unique_ptr<WidgetBase> (*ctor)(WidgetManager &, WidgetBase *, const WidgetDescriptor *) = nullptr, const Serialize::SerializeTable *serializeTable = nullptr, WidgetTemplate _template = {})
            : mSerializeTable(serializeTable)
            , mCtor(ctor)
            , mTemplate(std::move(_template))
        {
        }
        template <typename WidgetType>
        WidgetDescriptor(type_holder_t<WidgetType>)
            : mCtor([](WidgetManager &manager, WidgetBase *parent, const WidgetDescriptor *desc) -> std::unique_ptr<WidgetBase> {
                return std::make_unique<WidgetType>(manager, parent);
            })
            , mSerializeTable(&::serializeTable<WidgetType>())
        {
        }

        std::unique_ptr<WidgetBase> create(WidgetManager &manager, WidgetBase *parent = nullptr) const;
        const Serialize::SerializeTable *serializeTable();
        const WidgetTemplate &widgetTemplate() const;

    private:
        const Serialize::SerializeTable *mSerializeTable = nullptr;
        std::unique_ptr<WidgetBase> (*mCtor)(WidgetManager &, WidgetBase *, const WidgetDescriptor *) = nullptr;

        WidgetTemplate mTemplate;
    };

    struct MADGINE_WIDGETS_EXPORT WidgetLoader : Resources::ResourceLoader<WidgetLoader, WidgetDescriptor> {
        WidgetLoader();

        Threading::Task<bool> init() override;

        Threading::Task<bool> loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(WidgetDescriptor &descriptor);
    };

}
}

REGISTER_TYPE(Engine::Widgets::WidgetLoader)