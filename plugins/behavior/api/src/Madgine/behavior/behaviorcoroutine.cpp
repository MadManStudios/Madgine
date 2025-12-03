#include "../behaviorlib.h"

#include "behaviorcoroutine.h"

#include "behavior.h"

#include "Meta/keyvalue/valuetype.h"

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
            if (!mResolveNames(*mReceiver)) {
                throw 0;
            }
        }
        Execution::get_debug_location(*mReceiver)->stepInto(mDebugLocation);
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
    }

    void CoroutineBehaviorState::stop()
    {
    }

    void CoroutineBehaviorState::destroy()
    {
        std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).destroy();
    }

    void CoroutineBehaviorState::visitState(CallableView<void(const Execution::StateDescriptor &)> visitor)
    {
        visitor(Execution::State::SubLocation {mDebugLocation});
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
        Execution::get_debug_location(*mReceiver)->stepOut(mDebugLocation);
        // mValue = void
    }

    void CoroutineBehaviorState::unhandled_exception()
    {
        throw;
    }

    void CoroutineBehaviorState::set_error(BehaviorError result)
    {
        Execution::get_debug_location(*mReceiver)->stepOut(mDebugLocation);
        mReceiver->set_error(result);
    }

    void CoroutineBehaviorState::set_done()
    {
        Execution::get_debug_location(*mReceiver)->stepOut(mDebugLocation);
        mReceiver->set_done();
    }

    bool CoroutineBehaviorState::InitialSuspend::await_ready() noexcept
    {
        return false;
    }

    void CoroutineBehaviorState::InitialSuspend::await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept
    {
#ifndef NDEBUG
        handle.promise().mDebugLocation.mStacktrace = Debug::StackTrace<1>::getCurrent(1);
#endif
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

    void CoroutineBehaviorState::resume()
    {

        if (!mNext->resumeImpl()) {
            set_error(BehaviorError {});
        }
    }

    void CoroutineBehaviorState::suspend()
    {
        mNext->suspendImpl();
    }

    bool CoroutineBehaviorState::resumeImpl()
    {
        return true;
    }

    void CoroutineBehaviorState::suspendImpl()
    {
    }

    std::string CoroutineLocation::toString() const
    {
#ifdef NDEBUG
        return "<Coroutine>";
#else
        return mStacktrace.calculateReadable()[0].mFunction;
#endif
    }

    std::map<std::string_view, ValueType> CoroutineLocation::localVariables() const
    {
        return {};
    }

    bool CoroutineLocation::wantsPause(Debug::ContinuationType type, IndexType<size_t> line) const
    {
        return type == Debug::ContinuationType::Error || Debug::DebugLocation::wantsPause(type, line);
    }


}
}