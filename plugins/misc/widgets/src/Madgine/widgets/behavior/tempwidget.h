#pragma once

#include "../widgetloader.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Widgets {

    Behavior tempWidget(WidgetLoader::Handle desc, Behavior behavior);

    struct TempWidgetState : BehaviorReceiver {
        TempWidgetState(WidgetLoader::Handle desc, std::vector<Behavior> behaviors);
        ~TempWidgetState();

        void start();

    private:
        WidgetLoader::Handle mDesc;
        std::vector<Behavior> mBehaviors;
        std::unique_ptr<WidgetBase> mWidget;
    };

}
}

NATIVE_BEHAVIOR_DECLARATION(temp_widget)