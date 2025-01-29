#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/sender.h"
#include "Generic/execution/storage.h"
#include "Generic/makeowning.h"
#include "Generic/withresult.h"

#include "Madgine/debug/debuggablesender.h"

namespace Engine {

template <typename Sender>
struct BehaviorAwaitableSender;

template <typename Sender>
struct BehaviorAwaitableReceiver : Execution::execution_receiver<> {

    template <typename... V>
    void set_value(V &&...value)
    {
        mState->set_value(std::forward<V>(value)...);
    }

    void set_done()
    {
        mState->set_done();
    }

    template <typename... R>
    void set_error(R &&...result)
    {
        mState->set_error(std::forward<R>(result)...);
    }

    template <typename CPO, typename... Args>
    requires(is_tag_invocable_v<CPO, BehaviorReceiver &, Args...>) friend auto tag_invoke(CPO f, BehaviorAwaitableReceiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
        -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
    {
        return tag_invoke(f, *rec.mBehavior->mReceiver, std::forward<Args>(args)...);
    }

    BehaviorAwaitableSender<Sender> *mState;
    CoroutineBehaviorState *mBehavior;
};

template <typename Sender>
struct BehaviorAwaitableSender {

    auto buildState(Sender &&sender, CoroutineBehaviorState *state)
    {
        return Execution::connect(std::forward<Sender>(sender) | Execution::with_debug_location(), BehaviorAwaitableReceiver<Sender> { {}, this, state });
    }

    using S = std::invoke_result_t<decltype(&BehaviorAwaitableSender::buildState), BehaviorAwaitableSender, Sender, nullptr_t>;

    BehaviorAwaitableSender(Sender &&sender, CoroutineBehaviorState *state)
        : mState(buildState(std::forward<Sender>(sender), state))
    {
    }

    bool await_ready()
    {
        mState.start();
        return mFlag.test() && mResult.is_value();
    }

    bool await_suspend(std::coroutine_handle<CoroutineBehaviorState> behavior)
    {
        mBehavior = behavior;
        if (mFlag.test_and_set()) {
            if (mResult.is_value()) {
                return false;
            } else if (mResult.is_error()) {
                mResult.reproduce_error(mBehavior.promise());                
            } else {
                mBehavior.promise().set_done();
            }
        } 
        return true;
    }

    Execution::ValueStorage<Sender> await_resume()
    {
        return std::move(mResult).value();
    }

    template <typename... V>
    void set_value(V &&...v)
    {
        mResult.set_value(std::forward<V>(v)...);
        if (mFlag.test_and_set())
            mBehavior.resume();
    }

    void set_done()
    {
        mResult.set_done();
        if (mFlag.test_and_set())
            mBehavior.promise().set_done();
    }

    template <typename... R>
    void set_error(R &&...error)
    {
        mResult.set_error(std::forward<R>(error)...);
        if (mFlag.test_and_set())
            mResult.reproduce_error(mBehavior.promise());
    }

private:
    S mState;
    std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
    std::coroutine_handle<CoroutineBehaviorState> mBehavior;
    Execution::ResultStorage<Sender> mResult;
};

}
