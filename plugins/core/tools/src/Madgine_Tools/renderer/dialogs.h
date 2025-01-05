#pragma once

#include "Generic/enum.h"
#include "Generic/execution/concepts.h"
#include "Generic/type_pack.h"


namespace Engine {
namespace Tools {

    ENUM(DialogResult,
        Accepted,
        Declined)

    struct DialogFlags {
        bool acceptPossible = true;
        bool implicitlyAccepted = false;
    };

    struct DialogStateBase;

    struct MADGINE_TOOLS_EXPORT DialogContainer {
        void push(DialogStateBase *dialog);
        void pop(DialogStateBase *dialog);

        void render();

        DialogStateBase *mHead = nullptr;
    };

    struct DialogSettings {
        bool showAccept = true;
        bool showDecline = false;
        bool showCancel = true;
        std::string acceptText = "Yes";
        std::string declineText = "No";
        std::string cancelText = "Cancel";

        std::string header = "";
    };

    struct MADGINE_TOOLS_EXPORT DialogStateBase {

        DialogStateBase(DialogSettings settings, DialogContainer *targetContainer);

        void start();

        void render();

    protected:
        virtual DialogFlags renderContent() = 0;
        virtual void close(DialogResult) = 0;
        virtual void cancel() = 0;

    private:
        DialogSettings mSettings;
        friend struct DialogContainer;
        DialogContainer *mTargetContainer;

        DialogStateBase *mNext = nullptr;
    };

    template <typename Rec, typename F, typename Tuple>
    struct DialogState : Execution::base_state<Rec>, DialogStateBase {

        DialogState(Rec &&rec, DialogSettings settings, DialogContainer *targetContainer, F &&f, Tuple &&tuple)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , DialogStateBase { std::move(settings), targetContainer }
            , mF(std::forward<F>(f))
            , mTuple(std::forward<Tuple>(tuple))
        {
        }

        DialogFlags renderContent() override
        {
            return TupleUnpacker::invokeExpand(mF, mTuple);
        }

        void close(DialogResult result) override
        {
            DialogStateBase::close(result);
            TupleUnpacker::invokeExpand(LIFT(this->mRec.set_value, this), result, std::move(mTuple));
        }

        void cancel() override
        {
            DialogStateBase::cancel();
            this->mRec.set_done();
        }

        F mF;
        Tuple mTuple;
    };

    template <typename F, typename Tuple>
    struct DialogSender {

        using is_sender = void;

        using result_type = void;
        template <template <typename...> typename Variant>
        using value_types = typename Engine::to_type_pack<Tuple>::template prepend<DialogResult>::template instantiate<Variant>;

        template <typename Rec>
        friend auto tag_invoke(Engine::Execution::connect_t, DialogSender &&sender, Rec &&rec)
        {
            return DialogState<Rec, F, Tuple> { std::forward<Rec>(rec), std::move(sender.mSettings), sender.mTargetContainer, std::forward<F>(sender.mF), std::forward<Tuple>(sender.mTuple) };
        }

        DialogSettings mSettings;
        DialogContainer *mTargetContainer;
        F mF;
        Tuple mTuple;
    };

    template <typename F, typename Tuple>
    DialogSender(DialogSettings, DialogContainer *, F &&, Tuple &&) -> DialogSender<F, Tuple>;

}
}
