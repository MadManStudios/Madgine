#include "../behaviorlib.h"

#include "behavior.h"

namespace Engine {
namespace Behavior {

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

    void tag_invoke(Execution::visit_state_t, Behavior::state *state, BehaviorStateBase::CB visitor)
    {
        if (state)
            state->mState->visitState(std::move(visitor));
        else
            visitor(Execution::State::Text { "Behavior" });
    }

    Behavior::StatePtr Behavior::connect(BehaviorReceiver &receiver)
    {
        mState->connect(receiver);
        return std::move(mState);
    }

}
}