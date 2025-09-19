#pragma once

#include "Madgine/resources/resourceloader.h"
#include "Meta/serialize/hierarchy/serializetable_forward.h"

namespace Engine {
namespace Widgets {

    
    struct MADGINE_WIDGETS_EXPORT WidgetLoader : Resources::ResourceLoader<WidgetLoader, WidgetDescriptor> {
        WidgetLoader();

        struct MADGINE_WIDGETS_EXPORT Handle : Base::Handle {
            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            std::unique_ptr<WidgetBase> create(WidgetManager &manager, WidgetBase *parent = nullptr) const;
        };

        Threading::Task<bool> init() override;

        Threading::Task<bool> loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(WidgetDescriptor &descriptor);
    };

    struct MADGINE_WIDGETS_EXPORT WidgetDescriptor {
        WidgetDescriptor();

        WidgetDescriptor(std::unique_ptr<WidgetTemplate> _template);
        template <typename WidgetType>
        WidgetDescriptor(type_holder_t<WidgetType>)
            : mCtor([](WidgetManager &manager, WidgetBase *parent, WidgetLoader::Handle desc) -> std::unique_ptr<WidgetBase> {
                return std::make_unique<WidgetType>(manager, parent);
            })
            , mSerializeTable(&::serializeTable<WidgetType>())
            , mMetaTable(table<WidgetType>)
        {
        }
        ~WidgetDescriptor();

        WidgetDescriptor &operator=(WidgetDescriptor &&) noexcept;

        const MetaTable *metaTable();
        const Serialize::SerializeTable *serializeTable();

        const std::unique_ptr<WidgetTemplate> &widgetTemplate() const;
        std::unique_ptr<WidgetBase> create(WidgetManager &manager, WidgetLoader::Handle handle, WidgetBase *parent = nullptr) const;

    private:
        const Serialize::SerializeTable *mSerializeTable = nullptr;
        const MetaTable *mMetaTable = nullptr;

        std::unique_ptr<WidgetBase> (*mCtor)(WidgetManager &, WidgetBase *, WidgetLoader::Handle) = nullptr;

        std::unique_ptr<WidgetTemplate> mTemplate;
    };


}
}