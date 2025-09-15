#pragma once

#include "widget.h"

#include "Meta/keyvalue/metatable.h"
#include "Meta/serialize/hierarchy/serializetable.h"

#include "widgetloader.h"

namespace Engine {
namespace Widgets {

    struct WidgetData {
        std::string mName;
        std::string mType;
    };

    struct MADGINE_WIDGETS_EXPORT WidgetTemplate {

        WidgetTemplate(std::string name, std::vector<WidgetData> widgets);
        ~WidgetTemplate();

        static std::unique_ptr<Serialize::Serializer[]> serializers(const std::vector<WidgetData> &widgets);
        static std::unique_ptr<Accessor[]> accessors(const std::vector<WidgetData> &widgets);

        std::string mName;
        std::vector<WidgetData> mWidgets;

        std::unique_ptr<Serialize::Serializer[]> mSerializers;
        Serialize::SerializeTable mSerializeTable;
        std::unique_ptr<Accessor[]> mAccessors;
        const MetaTable *mSelfTable = &mMetaTable;
        MetaTable mMetaTable;
    };

    struct MADGINE_WIDGETS_EXPORT CompoundWidget : WidgetBase {

        CompoundWidget(WidgetManager &mgr, WidgetLoader::Handle desc, WidgetBase *parent = nullptr);

        const char *getClass() const override;

        void render(WidgetsRenderData &renderData) override;
        void updateChildrenGeometry() override;

        const std::vector<std::unique_ptr<WidgetBase>> &templateWidgets() const;

        WidgetBase *getTemplateWidget(std::string_view name);
        const WidgetBase *getTemplateWidget(std::string_view name) const;
        template <typename T>
        T *getTemplateWidget(std::string_view name)
        {
            return dynamic_cast<T *>(getTemplateWidget(name));
        }

        Serialize::SerializableDataPtr customUnitPtr() override
        {
            return { this, mDescriptor->serializeTable() };
        }
        Serialize::SerializableDataConstPtr customUnitPtr() const override
        {
            return { this, mDescriptor->serializeTable() };
        }

        virtual ScopePtr customScopePtr() override
        {
            return { this, mDescriptor->metaTable() };
        }

    private:
        std::vector<std::unique_ptr<WidgetBase>> mTemplateWidgets;
        WidgetLoader::Handle mDescriptor;
    };

}
}