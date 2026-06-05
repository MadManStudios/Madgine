#pragma once

#include "Meta/reflect/metatable.h"

namespace Engine {
namespace Widgets {

    struct WidgetData {
        std::string mName;
        std::string mType;
    };

    struct MADGINE_WIDGETS_EXPORT WidgetTemplate {

        WidgetTemplate(std::string name, std::vector<WidgetData> widgets);
        ~WidgetTemplate();

        static std::unique_ptr<Reflect::Accessor[]> accessors(const std::vector<WidgetData> &widgets);

        std::string mName;
        std::vector<WidgetData> mWidgets;

        std::unique_ptr<Reflect::Accessor[]> mAccessors;
        const Reflect::MetaTable *mSelfTable = &mMetaTable;
        Reflect::MetaTable mMetaTable;
    };

}
}