#pragma once

#include "../behavior.h"

namespace Engine {
namespace Behavior {

    template <typename Binding>
    struct BoundValue : BoundValueBase {

        using T = typename std::decay_t<Binding>::type;

        explicit BoundValue(Binding &&binding, CoroutineBehaviorState *state)
            : BoundValueBase(state)
            , mBinding(std::forward<Binding>(binding))
        {
            [[maybe_unused]] bool success = Execution::access_binding(mBinding, [this](auto &&v) {
                mValue.emplace(std::forward<decltype(v)>(v));
            });
            assert(success);
        }
        BoundValue(BoundValue &&) = delete;

        std::conditional_t<std::is_pointer_v<T>, T, std::remove_reference_t<T> *> operator->()
        {
            if constexpr (std::is_pointer_v<T>) {
                return *mValue;
            } else if constexpr (std::is_reference_v<T>) {
                return &mValue->get();
            } else {
                return &*mValue;
            }
        }

        bool resumeImpl() override
        {
            return Execution::access_binding(mBinding, [this](auto &&v) {
                mValue.emplace(std::forward<decltype(v)>(v));
                return mNext->resumeImpl();
            });
        }

        void suspendImpl() override
        {
            mNext->suspendImpl();
            mValue.reset();
        }

        std::optional<forward_ref_t<T>> mValue;
        Binding mBinding;
    };

    template <typename Binding>
    struct BehaviorAwaitableBinding {

        BehaviorAwaitableBinding(Binding &&binding)
            : mBinding(std::forward<Binding>(binding))
        {
        }

        bool await_ready()
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<CoroutineBehaviorState> behavior)
        {
            mState = &behavior.promise();
            if (!Execution::access_binding(mBinding, [&](const auto &) {
                    behavior.resume();
                })) {
                behavior.promise().set_error(KeyValueError { GenericResult::UNKNOWN_ERROR, "TODO" });
            }
        }

        BoundValue<Binding> await_resume()
        {
            return BoundValue<Binding> { std::forward<Binding>(mBinding), mState };
        }

    private:
        CoroutineBehaviorState *mState;
        Binding mBinding;
    };

}
}