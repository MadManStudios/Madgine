#include "behaviorlib.h"

#include "behavior.h"

#include "Meta/keyvalue/valuetype.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::BehaviorStateBase)
METATABLE_END(Engine::BehaviorStateBase)

namespace Engine {

void Behavior::destroyState(BehaviorStateBase *state)
{
    state->destroy();
}

Behavior::Behavior(StatePtr state)
    : mState(std::move(state))
{
}

Behavior &Behavior::operator=(StatePtr state)
{
    mState = std::move(state);
    return *this;
}

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
    mDebugLocation.stepInto(Execution::get_debug_location(*mReceiver));
    std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
}

void CoroutineBehaviorState::stop()
{
    throw 0;
}

void CoroutineBehaviorState::destroy()
{
    std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).destroy();
}

void CoroutineBehaviorState::visitState(CallableView<void(const Execution::StateDescriptor &)> visitor)
{
    visitor(Execution::State::Text { "TODO" });
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
    mDebugLocation.stepOut(Execution::get_debug_location(*mReceiver));
    // mValue = void
}

void CoroutineBehaviorState::unhandled_exception()
{
    throw;
}

void CoroutineBehaviorState::set_error(BehaviorError result)
{
    mDebugLocation.stepOut(Execution::get_debug_location(*mReceiver));
    mReceiver->set_error(result);
}

void CoroutineBehaviorState::set_done()
{
    mDebugLocation.stepOut(Execution::get_debug_location(*mReceiver));
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

Behavior::state::state(StatePtr state)
    : mState(std::move(state))
{
}

void Behavior::state::start()
{
    mState->start();
}

void Behavior::state::stop()
{
    mState->stop();
}

void Behavior::state::connect()
{
    mState->connect(*this);
}

void tag_invoke(Execution::visit_state_t, Behavior::state *state, CallableView<void(const Execution::StateDescriptor &)> visitor)
{
    if (state)
        state->mState->visitState(visitor);
    else
        visitor(Execution::State::Text { "Behavior" });
}

Behavior::StatePtr Behavior::connect(BehaviorReceiver &receiver)
{
    mState->connect(receiver);
    return std::move(mState);
}

}