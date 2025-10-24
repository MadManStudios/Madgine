#pragma once

#include "Generic/enum.h"

#include "widgetloader.h"

namespace Engine {
namespace Widgets {

    ENUM(WidgetType,
        DEFAULT_WIDGET,
        MODAL_OVERLAY,
        NONMODAL_OVERLAY,
        ROOT_WIDGET);

    struct LayoutWidget {
        std::string mName;
        WidgetLoader::Handle mWidgetTemplate;
        Execution::Flag<WidgetBase *> mWidget;
        WidgetType mType = WidgetType::DEFAULT_WIDGET;
        bool mDefaultVisibility = false;
    };

}
}