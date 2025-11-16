#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/sender.h"
#include "Generic/execution/storage.h"
#include "Generic/makeowning.h"
#include "Generic/withresult.h"

#include "../behavior.h"

#include "Madgine/debug/debuggablesender.h"

namespace Engine {

template <typename Binding>
struct BoundValue : BoundValueBase {

    using T = typename std::decay_t<Binding>::type;

    explicit BoundValue(Binding &&binding, CoroutineBehaviorState *state)
        : BoundValueBase(state)
        , mBinding(std::forward<Binding>(binding))
    {
        bool success = Execution::access_binding(mBinding, [this](auto &&v) {
            construct(mValue, std::forward<decltype(v)>(v));
        });
        assert(success);
    }
    BoundValue(BoundValue &&) = delete;
    ~BoundValue()
    {
        destruct(mValue);
    }

    std::conditional_t<std::is_pointer_v<T>, T, std::remove_reference_t<T> *> operator->()
    {
        if constexpr (std::is_pointer_v<T>) {
            return mValue;
        } else {
            return &mValue;
        }
    }

    bool resumeImpl() override
    {
        return Execution::access_binding(mBinding, [this](auto &&v) {
            construct(mValue, std::forward<decltype(v)>(v));
            return mNext->resumeImpl();
        });
    }

    void suspendImpl() override
    {
        mNext->suspendImpl();
        destruct(mValue);
    }

    ManualLifetime<T> mValue
        = std::nullopt;
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
            behavior.promise().set_error(BehaviorError {});
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
