#pragma once

#include "Meta/serialize/hierarchy/serializetable_forward.h"

#include "Madgine/resources/resourceloader.h"

#include "widgettemplate.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT WidgetLoader : Resources::ResourceLoader<WidgetLoader, WidgetDescriptor> {
        WidgetLoader();

        struct MADGINE_WIDGETS_EXPORT Handle : Base::Handle {

            Handle();

            Handle(Resource *res);

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
        WidgetDescriptor() = default;

        WidgetDescriptor(std::unique_ptr<WidgetTemplate> _template);
        ~WidgetDescriptor();

        WidgetDescriptor &operator=(WidgetDescriptor &&) noexcept;

        const Reflect::MetaTable *metaTable();

        const std::unique_ptr<WidgetTemplate> &widgetTemplate() const;
        std::unique_ptr<WidgetBase> create(WidgetManager &manager, WidgetLoader::Handle handle, WidgetBase *parent = nullptr) const;

    private:
        const Reflect::MetaTable *mMetaTable = nullptr;

        std::unique_ptr<WidgetBase> (*mCtor)(WidgetManager &, WidgetBase *, WidgetLoader::Handle) = nullptr;

        std::unique_ptr<WidgetTemplate> mTemplate;
    };

}
}