#include "../../widgetslib.h"

#include "tempwidget.h"

#include "Madgine/resources/sender.h"

#include "../widget.h"
#include "../widgetmanager.h"

#include "Madgine/window/mainwindow.h"

#include "../tablewidget.h"

namespace Engine {
namespace Widgets {

    template <typename Rec>
    struct TempWidgetStateImpl : VirtualBehaviorState<Rec, TempWidgetState> {

        friend auto tag_invoke(Execution::visit_state_t, TempWidgetStateImpl &state, const auto &, auto &&visitor)
        {
            visitor(Execution::State::BeginBlock { "Temp Widget" });

            visitor(Execution::State::Text { "Dummy" });

            visitor(Execution::State::EndBlock {});
        }

        using VirtualBehaviorState<Rec, TempWidgetState>::VirtualBehaviorState;
    };

    struct TempWidgetSender : Execution::base_sender {
        using result_type = BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, TempWidgetSender &&sender, Rec &&rec)
        {
            return TempWidgetStateImpl<Rec> { std::forward<Rec>(rec), std::move(sender.mDesc), std::move(sender.mBehavior) };
        }

        static constexpr size_t debug_start_increment = 1;
        static constexpr size_t debug_operation_increment = 1;
        static constexpr size_t debug_stop_increment = 1;

        WidgetLoader::Handle mDesc;
        Behavior mBehavior;
    };

    Behavior tempWidget(WidgetLoader::Handle desc, Behavior behavior)
    {
        return TempWidgetSender { {}, WidgetLoader::Handle { desc }, std::move(behavior) } | Resources::with_handle(WidgetLoader::Handle { desc });
    }

    TempWidgetState::TempWidgetState(WidgetLoader::Handle desc, Behavior behavior)
        : mDesc(std::move(desc))
        , mBehavior(std::move(behavior))
    {
    }

    TempWidgetState::~TempWidgetState()
    {
    }

    void TempWidgetState::start()
    {
        WidgetManager *mgr;
        BehaviorError error = get_binding<"WidgetManager">(*this, mgr);
        if (error.mResult != BehaviorResult::SUCCESS) {
            set_error(std::move(error));
            return;
        }

        mWidget = mDesc->create(*mgr);
        mgr->openOverlay(mWidget.get());
        mgr->lifetime().attach(std::move(mBehavior) | Execution::then([this, mgr]() {
            mgr->closeOverlay(mWidget.get());
            set_value();
        }));
    }

}
}

NATIVE_BEHAVIOR(temp_widget, Engine::Widgets::tempWidget, Engine::InputParameter<Engine::Widgets::WidgetLoader::Handle>, Engine::SubBehavior)