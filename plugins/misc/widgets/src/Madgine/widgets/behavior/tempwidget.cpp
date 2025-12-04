#include "../../widgetslib.h"

#include "tempwidget.h"

#include "Madgine/behavior/nativebehaviorcollector.h"
#include "Madgine/resources/sender.h"
#include "Madgine/window/mainwindow.h"

#include "../tablewidget.h"
#include "../widget.h"
#include "../widgetmanager.h"

namespace Engine {
namespace Widgets {

    struct TempWidgetSender : Execution::base_sender {
        using result_type = Behavior::BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, TempWidgetSender &&sender, Rec &&rec)
        {
            return Behavior::VirtualBehaviorState<Rec, TempWidgetState> { std::forward<Rec>(rec), std::move(sender.mDesc), std::move(sender.mPos), std::move(sender.mSize), std::move(sender.mBehavior) };
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, TempWidgetSender &sender, Rec &&rec) -> Behavior::VirtualBehaviorState<Rec, TempWidgetState>
        {
            throw 0;
            // return Behavior::VirtualBehaviorState<Rec, TempWidgetState> { std::forward<Rec>(rec), sender.mDesc, sender.mPos, sender.mSize, sender.mBehavior };
        }

        WidgetLoader::Handle mDesc;
        Matrix3 mPos;
        Matrix3 mSize;
        Behavior::Behavior mBehavior;
    };

    Behavior::Behavior tempWidget(WidgetLoader::Handle desc, const Matrix3 &pos, const Matrix3 &size, Behavior::Behavior behavior)
    {
        return TempWidgetSender { {}, WidgetLoader::Handle { desc }, pos, size, std::move(behavior) } | Resources::with_handle(WidgetLoader::Handle { desc });
    }

    TempWidgetState::TempWidgetState(WidgetLoader::Handle desc, Matrix3 pos, Matrix3 size, Behavior::Behavior behavior)
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
        bool result = Behavior::get_named<"WidgetManager", WidgetManager *>(*this, mgr);
        assert(result);

        assert(!mWidget);
        mWidget = mDesc.create(*mgr);
        mWidget->setPos(mPos);
        mWidget->setSize(mSize);
        mgr->openOverlay(mWidget.get());
        mState.start();
    }

    void TempWidgetState::stop()
    {
        mState.stop();
    }

    WidgetBase *TempWidgetState::widget()
    {
        return mWidget.get();
    }

    void TempWidgetState::receiver::set_value(ArgumentList args)
    {
        WidgetManager *mgr;
        bool result = Behavior::get_named<"WidgetManager", WidgetManager *>(*this, mgr);
        assert(result);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_value(std::move(args));
    }

    void TempWidgetState::receiver::set_error(Behavior::BehaviorError error)
    {
        WidgetManager *mgr;
        bool result = Behavior::get_named<"WidgetManager", WidgetManager *>(*this, mgr);
        assert(result);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_error(std::move(error));
    }

    void TempWidgetState::receiver::set_done()
    {
        WidgetManager *mgr;
        bool result = Behavior::get_named<"WidgetManager", WidgetManager *>(*this, mgr);
        assert(result);

        mgr->closeOverlay(mState.mWidget.get());
        algorithm_receiver::set_done();
    }

}
}

NATIVE_BEHAVIOR(temp_widget, Engine::Widgets::tempWidget, Engine::Behavior::InputParameter<"Class", Engine::Widgets::WidgetLoader::Handle>, Engine::Behavior::InputParameter<"Position", Engine::Matrix3>, Engine::Behavior::InputParameter<"Size", Engine::Matrix3>, Engine::Behavior::SubBehavior)