#pragma once

#include "behaviorerror.h"

#include "Interfaces/debug/stacktrace.h"

#include "Generic/delayedconstruct.h"

#include "Madgine/debug/debuggablesender.h"

#include "behaviorreceiver.h"

#include "Generic/functor.h"

namespace Engine {

template <typename T>
concept UntypedBehavior = Execution::Sender<T>;

template <typename T, typename R>
concept TypedBehavior = UntypedBehavior<T>;

template <typename Algorithm>
struct SenderBehaviorState;

struct CoroutineBehaviorState;

struct MADGINE_BEHAVIOR_EXPORT Behavior {

    static void destroyState(BehaviorStateBase *state);
    using StatePtr = std::unique_ptr<BehaviorStateBase, Functor<&destroyState>>;

    Behavior() = default;
    Behavior(StatePtr state);

    template <typename Sender>
    Behavior(Sender &&sender)
        : mState(new SenderBehaviorState<Sender>(std::forward<Sender>(sender)))
    {
    }

    Behavior &operator=(StatePtr state);

    StatePtr release()
    {
        return std::move(mState);
    }

    StatePtr connect(BehaviorReceiver &receiver);

    struct MADGINE_BEHAVIOR_EXPORT state : BehaviorReceiver {

        state(StatePtr state);

        void start();

        StatePtr mState;
    };

    using is_sender = void;

    using result_type = BehaviorError;
    template <template <typename...> typename Tuple>
    using value_types = Tuple<ArgumentList>;

    template <typename Rec, std::same_as<Behavior> T> //Necessary to prevent implicit conversion
    friend auto tag_invoke(Execution::connect_t, T &&behavior, Rec &&rec)
    {
        assert(behavior.mState);
        return VirtualBehaviorState<Rec, state> { std::forward<Rec>(rec), std::move(behavior.mState) };
    }

    using promise_type = CoroutineBehaviorState;

    StatePtr mState;
};

struct BehaviorStateBase {
    virtual ~BehaviorStateBase() = default;

    virtual void connect(BehaviorReceiver &rec) = 0;
    virtual void start() = 0;
    virtual void destroy()
    {
        delete this;
    }    
};

template <typename Sender>
struct BehaviorAwaitableSender;

struct MADGINE_BEHAVIOR_EXPORT CoroutineLocation : Debug::DebugLocation {

    std::string toString() const override;
    std::map<std::string_view, ValueType> localVariables() const override;
    virtual bool wantsPause(Debug::ContinuationType type) const override;

#ifndef NDEBUG
    Debug::StackTrace<1> mStacktrace;
#endif
};

struct MADGINE_BEHAVIOR_EXPORT CoroutineBehaviorState : BehaviorStateBase {

    Behavior get_return_object();

    void connect(BehaviorReceiver &rec) override;
    void start() override;
    void destroy() override;

    struct MADGINE_BEHAVIOR_EXPORT InitialSuspend {
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept;
        void await_resume() noexcept;
    };

    struct MADGINE_BEHAVIOR_EXPORT FinalSuspend {
        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept;
        void await_resume() noexcept;
    };

    InitialSuspend initial_suspend() noexcept;
    FinalSuspend final_suspend() noexcept;

    void return_void();
    void unhandled_exception();
    void set_error(BehaviorError result);
    void set_done();

    template <typename T>
    decltype(auto) await_transform(T &&awaitable)
    {
        if constexpr (Execution::Sender<std::remove_reference_t<T>>) {
            return BehaviorAwaitableSender<T> { std::forward<T>(awaitable), this };
        } else {
            return std::forward<T>(awaitable);
        }
    }

    CoroutineLocation mDebugLocation;

    BehaviorReceiver *mReceiver = nullptr;
};

template <typename Sender>
struct SenderBehaviorState : BehaviorStateBase {

    using State = Execution::connect_result_t<typename Execution::with_debug_location_t::sender<Sender>, BehaviorReceiver&>;

    SenderBehaviorState(Sender &&sender)
        : mData(std::forward<Sender>(sender))
    {
    }

    void connect(BehaviorReceiver &rec) override
    {
        Sender sender = std::forward<Sender>(std::get<Sender>(mData));
        mData.template emplace<State>(
            DelayedConstruct<State> { [&]() { return Execution::connect(std::forward<Sender>(sender) | Execution::with_debug_location(), rec); } });
    }

    void start() override
    {
        std::get<State>(mData).start();
    }

    std::variant<Sender, State> mData;
};
}

REGISTER_TYPE(Engine::BehaviorStateBase);