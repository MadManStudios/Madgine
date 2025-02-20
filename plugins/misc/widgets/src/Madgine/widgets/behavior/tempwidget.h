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

        WidgetBase *widget();

    private:
        WidgetLoader::Handle mDesc;

        struct receiver : Execution::algorithm_receiver<BehaviorReceiver &> {
            void set_value(ArgumentList args);
            void set_error(BehaviorError error);
            void set_done();

            friend BehaviorError tag_invoke(get_binding_d_t, receiver& rec, std::string_view name, ValueTypeRef& out) {
                if (name == "Widget") {
                    out = rec.mState.widget();
                    return {};
                } else {
                    return get_binding_d(rec.mRec, name, out);
                }
            }

            TempWidgetState &mState;
        };

        using state = Execution::connect_result_t<Behavior, receiver>;
        state mState;

        std::unique_ptr<WidgetBase> mWidget;
    };

}
}

NATIVE_BEHAVIOR_DECLARATION(temp_widget)