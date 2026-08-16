#include "../behaviorlib.h"

#include "behaviorcoroutine.h"

#include "behavior.h"

namespace Engine {
namespace Behavior {

    Behavior CoroutineBehaviorState::get_return_object()
    {
        return Behavior::StatePtr { this };
    }

    void CoroutineBehaviorState::connect(BehaviorReceiver &rec)
    {
        mReceiver = &rec;
    }

    void CoroutineBehaviorState::start()
    {
        if (mResolveNames) {
            Reflect::Result result = mResolveNames(*mReceiver);
            if (result) {
                mReceiver->set_error(std::move(*result.mError));
                return;
            }
        }
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
    }

    void CoroutineBehaviorState::stop()
    {
        mDebugLocation.mContinuation.stop();
    }

    void CoroutineBehaviorState::destroy()
    {
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).destroy();
    }

    void CoroutineBehaviorState::visitState(CB visitor)
    {
        visitor(Execution::State::DebugLocation { &mDebugLocation });
    }

    bool CoroutineBehaviorState::wantsPause()
    {
        return Debug::get_debug_context(*mReceiver).wantsPause(&mDebugLocation, Debug::ContinuationType::Flow, mDebugLocation.mLine);
    }

    void CoroutineBehaviorState::pass(Closure<void()> callback)
    {
        Debug::ContextInfo &context = Debug::get_debug_context(*mReceiver);
        if (wantsPause())
            mDebugLocation.mContinuation = context.suspend(&mDebugLocation, { [this, callback { std::move(callback) }](Debug::ContinuationMode mode) mutable {
                switch (mode) {
                case Debug::ContinuationMode::Continue:
                    callback();
                    break;
                case Debug::ContinuationMode::Abort:
                    mReceiver->set_done();
                    break;
                default:
                    throw 0;
                } }, Debug::ContinuationType::Flow });
        else
            callback();
    }

    CoroutineBehaviorState::InitialSuspend CoroutineBehaviorState::initial_suspend() noexcept
    {
        return {};
    }

    bool CoroutineBehaviorState::FinalSuspend::await_ready() noexcept
    {
        return false;
    }

    void CoroutineBehaviorState::FinalSuspend::await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept
    {
        handle.promise().mReceiver->set_value();
    }

    void CoroutineBehaviorState::FinalSuspend::await_resume() noexcept
    {
        std::terminate();
    }

    CoroutineBehaviorState::FinalSuspend CoroutineBehaviorState::final_suspend() noexcept
    {
        return {};
    }

    void CoroutineBehaviorState::return_void()
    {
        // mValue = void
    }

    void CoroutineBehaviorState::unhandled_exception()
    {
        throw;
    }

    std::bool_constant<true> CoroutineBehaviorState::set_error(Reflect::Error result)
    {
        mReceiver->set_error(result);
        return {};
    }

    std::bool_constant<true> CoroutineBehaviorState::set_done()
    {
        mReceiver->set_done();
        return {};
    }

    bool CoroutineBehaviorState::InitialSuspend::await_ready() noexcept
    {
        return false;
    }

    void CoroutineBehaviorState::InitialSuspend::await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle, std::source_location location) noexcept
    {
        handle.promise().mDebugLocation.mLocation = std::move(location);
    }

    void CoroutineBehaviorState::InitialSuspend::await_resume() noexcept
    {
    }

    BoundValueBase::BoundValueBase(CoroutineBehaviorState *state)
    {
        if (state != this) {
            BoundValueBase *tail = state;
            while (tail->mNext != state)
                tail = tail->mNext;
            tail->mNext = this;
        }
        mNext = state;
    }

    BoundValueBase::~BoundValueBase()
    {
        if (mNext != this) {
            BoundValueBase *prev = this;
            while (prev->mNext != this) {
                prev = prev->mNext;
            }
            prev->mNext = mNext;
        }
    }

    bool CoroutineBehaviorState::resumeImpl()
    {
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
        return true;
    }

    void BehaviorCoroutineHandle::resume() const
    {
        if (!promise().mNext->resumeImpl()) {
            promise().set_error(Reflect::Error { GenericResult { GenericResult::UNKNOWN_ERROR }, "A bound object has become unavailable" });
        }
    }

    BehaviorCoroutineHandle::BehaviorCoroutineHandle(std::coroutine_handle<CoroutineBehaviorState> handle)
        : mHandle(handle)
    {
    }

    CoroutineBehaviorState &BehaviorCoroutineHandle::promise() const
    {
        return mHandle.promise();
    }

}
}