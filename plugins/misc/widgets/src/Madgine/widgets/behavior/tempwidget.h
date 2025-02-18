#pragma once

#include "../widgetloader.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Widgets {

    Behavior tempWidget(WidgetLoader::Handle desc, Behavior behavior);

    struct TempWidgetState : BehaviorReceiver {

        TempWidgetState(WidgetLoader::Handle desc, Behavior behavior);
        ~TempWidgetState();

        void start();

    private:
        WidgetLoader::Handle mDesc;

        struct receiver : Execution::algorithm_receiver<BehaviorReceiver &> {
            void set_value(ArgumentList args);
            void set_error(BehaviorError error);
            void set_done();

            TempWidgetState &mState;
        };

        using state = Execution::connect_result_t<Behavior, receiver>;
        state mState;

        std::unique_ptr<WidgetBase> mWidget;
    };

}
}

NATIVE_BEHAVIOR_DECLARATION(temp_widget)