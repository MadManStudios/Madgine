#pragma once

#include "Generic/execution/statedescriptor.h"

#include "Madgine/root/root.h"

namespace Engine {
namespace Resources {

    struct with_handle_t {

        template <typename Rec, typename Handle>
        struct receiver : Execution::algorithm_receiver<Rec> {
            receiver(Rec &&rec, Handle &&handle)
                : Execution::algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mHandle(std::forward<Handle>(handle))
            {
            }

            Handle mHandle;
        };

        template <typename Sender, typename Handle, typename _Rec>
        struct state : Execution::base_state<receiver<_Rec, Handle>> {

            using InnerRec = _Rec;
            using Rec = receiver<InnerRec, Handle>;

            using State = Execution::connect_result_t<Sender, Rec &>;

            state(Sender &&sender, InnerRec &&rec, Handle &&handle)
                : Execution::base_state<Rec> { { std::forward<InnerRec>(rec), std::forward<Handle>(handle) } }
                , mState(Execution::connect(std::forward<Sender>(sender), this->mRec))
            {
            }
            state(state &&) = delete;

            void start()
            {
                auto handler = [this](bool success) {
                    if (success)
                        mState.start();
                    else
                        this->mRec.set_error(REFLECT_UNKNOWN_ERROR());
                };

                if (!this->mRec.mHandle) {
                    this->mRec.set_error(REFLECT_UNKNOWN_ERROR());
                    return;
                }

                Threading::TaskFuture<bool> fut = this->mRec.mHandle.info()->loadingTask();
                if (fut.is_ready()) {
                    handler(fut);
                } else {
                    Core::Root::getSingleton().taskQueue()->queueTask(fut.then(handler));
                }
            }

            void stop()
            {
                mState.stop();
            }

            friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
            {
                Execution::visit_state(state ? &state->mState : nullptr, std::forward<decltype(visitor)>(visitor));
            }

            State mState;
        };

        template <typename Sender, typename Handle>
        struct sender : Execution::algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return state<Sender, Handle, Rec> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::forward<Handle>(sender.mHandle) };
            }

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &sender, Rec &&rec)
            {
                return state<Sender &, Handle, Rec> { sender.mSender, std::forward<Rec>(rec), Handle { sender.mHandle } };
            }

            Handle mHandle;
        };

        template <typename Sender, typename Handle>
        friend auto tag_invoke(with_handle_t, Sender &&inner, Handle &&handle)
        {
            return sender<Sender, Handle> { { {}, std::forward<Sender>(inner) }, std::forward<Handle>(handle) };
        }

        template <typename Sender, typename Handle>
            requires tag_invocable<with_handle_t, Sender, Handle>
        auto operator()(Sender &&sender, Handle &&handle) const
            noexcept(is_nothrow_tag_invocable_v<with_handle_t, Sender, Handle>)
                -> tag_invoke_result_t<with_handle_t, Sender, Handle>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<Handle>(handle));
        }

        template <typename Handle>
        auto operator()(Handle &&handle) const
        {
            return pipable_from_right(*this, std::forward<Handle>(handle));
        }
    };

    inline constexpr with_handle_t with_handle;

}
}