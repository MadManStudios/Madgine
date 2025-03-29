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
            return TempWidgetStateImpl<Rec> { std::forward<Rec>(rec), std::move(sender.mDesc), std::move(sender.mPos), std::move(sender.mSize), std::move(sender.mBehavior) };
        }

        static constexpr size_t debug_start_increment = 1;
        static constexpr size_t debug_operation_increment = 1;
        static constexpr size_t debug_stop_increment = 1;

        WidgetLoader::Handle mDesc;
        Matrix3 mPos;
        Matrix3 mSize;
        Behavior mBehavior;
    };

    Behavior tempWidget(WidgetLoader::Handle desc, const Matrix3 &pos, const Matrix3 &size, Behavior behavior)
    {
        return TempWidgetSender { {}, WidgetLoader::Handle { desc }, pos, size, std::move(behavior) } | Resources::with_handle(WidgetLoader::Handle { desc });
    }

    TempWidgetState::TempWidgetState(WidgetLoader::Handle desc, Matrix3 pos, Matrix3 size, Behavior behavior)
        : mDesc(std::move(desc))
        , mState(Execution::connect(std::move(behavior), receiver { { *this }, *this }))
        , mPos(std::move(pos))
        , mSize(std::move(size))        
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

        assert(!mWidget);
        mWidget = mDesc->create(*mgr);
        mWidget->setPos(mPos);
        mWidget->setSize(mSize);
        mgr->openOverlay(mWidget.get());
        mState.start();
    }

    WidgetBase *TempWidgetState::widget()
    {
        return mWidget.get();
    }

    void TempWidgetState::receiver::set_value(ArgumentList args)
    {
        WidgetManager *mgr;
        BehaviorError error = get_binding<"WidgetManager">(*this, mgr);
        assert(error.mResult == BehaviorResult::SUCCESS);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_value(std::move(args));
    }

    void TempWidgetState::receiver::set_error(BehaviorError error)
    {
        WidgetManager *mgr;
        BehaviorError result = get_binding<"WidgetManager">(*this, mgr);
        assert(result.mResult == BehaviorResult::SUCCESS);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_error(std::move(error));
    }

    void TempWidgetState::receiver::set_done()
    {
        WidgetManager *mgr;
        BehaviorError error = get_binding<"WidgetManager">(*this, mgr);
        assert(error.mResult == BehaviorResult::SUCCESS);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_done();
    }

}
}

NATIVE_BEHAVIOR(temp_widget, Engine::Widgets::tempWidget, Engine::InputParameter<"Class", Engine::Widgets::WidgetLoader::Handle>, Engine::InputParameter<"Position", Engine::Matrix3>, Engine::InputParameter<"Size", Engine::Matrix3>, Engine::SubBehavior)