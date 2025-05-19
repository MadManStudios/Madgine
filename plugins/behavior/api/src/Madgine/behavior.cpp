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
    mDebugLocation.stepInto(Execution::get_debug_location(*mReceiver));
    std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).resume();
}

void CoroutineBehaviorState::destroy()
{
    std::coroutine_handle<CoroutineBehaviorState>::from_promise(*this).destroy();
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
    //mValue = void
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

bool CoroutineLocation::wantsPause(Debug::ContinuationType type) const
{
    return type == Debug::ContinuationType::Error || Debug::DebugLocation::wantsPause(type);
}

Behavior::state::state(StatePtr state)
    : mState(std::move(state))
{
    mState->connect(*this);
}

void Behavior::state::start()
{
    mState->start();
}

Behavior::StatePtr Behavior::connect(BehaviorReceiver &receiver)
{
    mState->connect(receiver);
    return std::move(mState);
}

}