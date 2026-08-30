#pragma once

#include "Meta/math/matrix3.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/context.h"

#include "../widgetloader.h"

namespace Engine {
namespace Widgets {

    MADGINE_WIDGETS_EXPORT Behavior::Behavior tempWidget(WidgetLoader::Handle desc, const Math::Matrix3 &pos, const Math::Matrix3 &size, Behavior::Behavior behavior);

    struct TempWidgetState : Behavior::BehaviorReceiver {

        TempWidgetState(WidgetLoader::Handle desc, Math::Matrix3 pos, Math::Matrix3 size, Behavior::Behavior behavior);
        ~TempWidgetState();

        void start();
        void stop();

        WidgetBase *widget();

    private:
        WidgetLoader::Handle mDesc;

        struct receiver : Execution::algorithm_receiver<Behavior::BehaviorReceiver &> {
            void set_value(Reflect::ArgumentList args);
            void set_error(Reflect::Error error);
            void set_done();

            friend Reflect::Result tag_invoke(Reflect::get_reflect_contextual_t, receiver &rec, Reflect::Value &out, const Reflect::MetaTable *type)
            {
                if (table<WidgetBase>->isDerivedFrom(type)){
                    toValue(out, rec.mState.widget());
                    return {};
                } else {
                    return Reflect::get_reflect_contextual(rec.mRec, out, type);
                }
            }

            TempWidgetState &mState;
        };

        using state = Execution::connect_result_t<Behavior::Behavior, receiver>;
        state mState;

        friend auto tag_invoke(Execution::visit_state_t, TempWidgetState *state, auto &&visitor)
        {
            visitor(Execution::State::BeginBlock { "Temp Widget" });

            Execution::visit_state(state ? &state->mState : nullptr, visitor);

            visitor(Execution::State::EndBlock {});
        }

        std::unique_ptr<WidgetBase> mWidget;
        Math::Matrix3 mPos;
        Math::Matrix3 mSize;
    };

}
}
