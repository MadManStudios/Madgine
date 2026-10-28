#include "../behaviorlib.h"

#include "behaviorcoroutine.h"

#include "behavior.h"

namespace Engine {
namespace Behavior {

    Behavior CoroutineBehaviorState::get_return_object()
    {
        return Behavior::StatePtr { this };
    }

    void CoroutineBehaviorState::start()
    {
        if (mResolveNames) {
            Reflect::Result result = mResolveNames(*mReceiver.mReceiver);
            if (result) {
                mDebugLocation.mContinuation.returnError(&mDebugLocation, mReceiver, {}, std::move(*result.mError));
                return;
            }
        }

        Debug::stepInto(mReceiver, &mDebugLocation);        

        mDebugLocation.mContinuation.start(mReceiver);
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
    }

    void CoroutineBehaviorState::stop()
    {
        if (mDebugLocation.mContinuation.stop())
            mDebugLocation.mContinuation.done();
    }

    void CoroutineBehaviorState::destroy()
    {
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).destroy();
    }

    bool CoroutineBehaviorState::wantsPause()
    {
        return Debug::get_debug_context(mReceiver).wantsPause(&mDebugLocation, Debug::ContinuationType::Flow, mDebugLocation.mLine);
    }

    void CoroutineBehaviorState::pass(Closure<void()> callback)
    {
        mDebugLocation.mContinuation.pass(&mDebugLocation, mReceiver, [this, callback { std::move(callback) }](auto &rec) { callback(); }, mDebugLocation.mLine);
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
        Debug::stepOut(handle.promise().mReceiver, &handle.promise().mDebugLocation);
        
        handle.promise().mDebugLocation.mContinuation.returnValue(&handle.promise().mDebugLocation, handle.promise().mReceiver);
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
        Debug::stepOut(mReceiver, &mDebugLocation);        

        mDebugLocation.mContinuation.returnError(&mDebugLocation, mReceiver, {}, std::move(result));
        return {};
    }

    std::bool_constant<true> CoroutineBehaviorState::set_done()
    {   
        Debug::stepOut(mReceiver, &mDebugLocation);        

        mDebugLocation.mContinuation.returnDone(&mDebugLocation, mReceiver);
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

    TypedPtr &tag_invoke(Debug::get_debug_child_location_t, CoroutineBehaviorState &state)
    {
        return state.mDebugLocation.mChild;
    }

}
}