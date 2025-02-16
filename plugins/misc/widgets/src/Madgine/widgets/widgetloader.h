#pragma once

#include "Madgine/resources/resourceloader.h"

namespace Engine {
namespace Widgets {

    struct WidgetDescriptor {
        WidgetDescriptor() = default;
        template <typename WidgetType>
        WidgetDescriptor(type_holder_t<WidgetType>)
            : mCtor([](WidgetManager &manager, WidgetBase *parent) -> std::unique_ptr<WidgetBase> {
                return std::make_unique<WidgetType>(manager, parent);
            })
        {
        }

        std::unique_ptr<WidgetBase> create(WidgetManager &manager, WidgetBase *parent = nullptr) const;
        const Serialize::SerializeTable *serializeTable();

    private:
        const Serialize::SerializeTable *mSerializeTable = nullptr;
        std::unique_ptr<WidgetBase> (*mCtor)(WidgetManager &manager, WidgetBase *) = nullptr;
    };

    struct MADGINE_WIDGETS_EXPORT WidgetLoader : Resources::ResourceLoader<WidgetLoader, WidgetDescriptor> {
        WidgetLoader();

        Threading::Task<bool> loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(WidgetDescriptor &descriptor);
    };

}
}

REGISTER_TYPE(Engine::Widgets::WidgetLoader)