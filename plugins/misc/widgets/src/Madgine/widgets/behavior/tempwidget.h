#pragma once

#include "../widgetloader.h"

#include "Madgine/behavior/behavior.h"

#include "Madgine/behavior/named.h"

#include "Meta/math/matrix3.h"

namespace Engine {
namespace Widgets {

    MADGINE_WIDGETS_EXPORT Behavior::Behavior tempWidget(WidgetLoader::Handle desc, const Matrix3 &pos, const Matrix3 &size, Behavior::Behavior behavior);

    struct TempWidgetState : Behavior::BehaviorReceiver {

        TempWidgetState(WidgetLoader::Handle desc, Matrix3 pos, Matrix3 size, Behavior::Behavior behavior);
        ~TempWidgetState();

        void start();
        void stop();

        WidgetBase *widget();

    private:
        WidgetLoader::Handle mDesc;

        struct receiver : Execution::algorithm_receiver<Behavior::BehaviorReceiver &> {
            void set_value(ArgumentList args);
            void set_error(Behavior::BehaviorError error);
            void set_done();

            friend bool tag_invoke(Behavior::get_named_d_t, receiver &rec, std::string_view name, ValueTypeRef &out)
            {
                if (name == "Widget") {
                    out = Execution::ConstantBinding { rec.mState.widget() };
                    return true;
                } else {
                    return Behavior::get_named_d(rec.mRec, name, out);
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
        Matrix3 mPos;
        Matrix3 mSize;
    };

}
}
