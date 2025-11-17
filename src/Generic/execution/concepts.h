#pragma once

#include "statedescriptor.h"

namespace Engine {
namespace Execution {

    template <typename T>
    concept Sender = requires {
        typename std::decay_t<T>::is_sender;
    };

    struct connect_t {
        template <typename Sender, typename Rec>
            requires tag_invocable<connect_t, Sender, Rec>
        auto operator()(Sender &&sender, Rec &&rec) const
            noexcept(is_nothrow_tag_invocable_v<connect_t, Sender, Rec>)
                -> tag_invoke_result_t<connect_t, Sender, Rec>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<Rec>(rec));
        }
    };

    struct outer_connect_t {
        template <typename Sender, typename Rec, typename = std::enable_if_t<!tag_invocable<outer_connect_t, Sender, Rec>>>
        auto operator()(Sender &&sender, Rec &&rec) const
            noexcept(is_nothrow_tag_invocable_v<connect_t, Sender, Rec>)
                -> tag_invoke_result_t<connect_t, Sender, Rec>
        {
            return tag_invoke(connect_t {}, std::forward<Sender>(sender), std::forward<Rec>(rec));
        }

        template <typename Sender, typename Rec>
            requires tag_invocable<outer_connect_t, Sender, Rec>
        auto operator()(Sender &&sender, Rec &&rec) const
            noexcept(is_nothrow_tag_invocable_v<outer_connect_t, Sender, Rec>)
                -> tag_invoke_result_t<outer_connect_t, Sender, Rec>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<Rec>(rec));
        }
    };

    inline constexpr outer_connect_t connect;

    template <typename Sender, typename Rec>
    using connect_result_t = decltype(connect(std::declval<Sender>(), std::declval<Rec>()));

    struct get_context_t {
        template <typename T>
            requires tag_invocable<get_context_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_context_t, T &>)
                -> tag_invoke_result_t<get_context_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_context_t get_context;

    struct unstoppable_token {
        operator StopToken()
        {
            return nullptr;
        }

        static constexpr bool stop_requested()
        {
            return false;
        }
    };

    struct get_stop_token_t {

        using signature = StopToken();

        template <typename T>
            requires(!tag_invocable<get_stop_token_t, T &>)
        auto operator()(T &) const
        {
            return unstoppable_token {};
        }

        template <typename T>
            requires tag_invocable<get_stop_token_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_stop_token_t, T &>)
                -> tag_invoke_result_t<get_stop_token_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_stop_token_t get_stop_token;

    template <typename T>
    concept has_stop_token = !std::same_as<decltype(get_stop_token(std::declval<T &>())), unstoppable_token>;

    struct CPU_context {
    };

    template <typename _Context = CPU_context>
    struct execution_receiver : _Context {
        using Context = _Context;

        friend Context &tag_invoke(Engine::Execution::get_context_t, execution_receiver &self)
        {
            return self;
        }
    };

    template <typename Rec>
    struct algorithm_receiver {

        template <typename... V>
        void set_value(V &&...value)
        {
            mRec.set_value(std::forward<V>(value)...);
        }

        void set_done()
        {
            mRec.set_done();
        }

        template <typename... R>
        void set_error(R &&...result)
        {
            mRec.set_error(std::forward<R>(result)...);
        }

        Rec mRec;

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, algorithm_receiver &rec, Args &&...args)
            -> tag_invoke_result_t<CPO, Rec &, Args...>
        {
            return f(rec.mRec, std::forward<Args>(args)...);
        }
    };

    template <typename Rec>
    struct base_state {

        base_state(Rec &&rec)
            : mRec(std::forward<Rec>(rec))
        {
        }

        template <typename... V>
        void set_value(V &&...value)
        {
            mRec.set_value(std::forward<V>(value)...);
        }
        void set_done()
        {
            mRec.set_done();
        }

        template <typename... R>
        void set_error(R &&...result)
        {
            mRec.set_error(std::forward<R>(result)...);
        }

        Rec mRec;
    };

    template <typename Sender, typename Rec>
    struct algorithm_state {

        algorithm_state(Sender &&sender, Rec &&rec)
            : mState { connect(std::forward<Sender>(sender), std::forward<Rec>(rec)) }
        {
        }

        ~algorithm_state() { }

        void start()
        {
            mState.start();
        }

        void stop()
        {
            mState.stop();
        }

        template <typename InnerRec, typename... V>
        void set_value(InnerRec &rec, V &&...value)
        {
            rec.set_value(std::forward<V>(value)...);
        }

        template <typename InnerRec>
        void set_done(InnerRec &rec)
        {
            rec.set_done();
        }

        template <typename InnerRec, typename... R>
        void set_error(InnerRec &rec, R &&...result)
        {
            rec.set_error(std::forward<R>(result)...);
        }

        friend auto tag_invoke(Execution::visit_state_t, algorithm_state *state, auto &&visitor)
        {
            return visit_state(state ? &state->mState : nullptr, std::forward<decltype(visitor)>(visitor));
        }

        connect_result_t<Sender, Rec> mState;
    };

    struct base_sender {
        using is_sender = void;
    };

    template <Sender Sender>
    struct algorithm_sender : base_sender {
        using result_type = typename std::decay_t<Sender>::result_type;
        template <template <typename...> typename Tuple>
        using value_types = typename std::decay_t<Sender>::template value_types<Tuple>;

        Sender mSender;
    };

    template <typename... T>
    struct signature : type_pack<T...> {
    };

    template <typename T>
    struct stream;

    template <typename T>
    using is_stream = is_instance<T, stream>;

    
    struct access_binding_t {

        template <typename T, typename F>
        auto operator()(T &&binding, F &&callback) const
            noexcept(is_nothrow_tag_invocable_v<access_binding_t, T, F>)
                -> tag_invoke_result_t<access_binding_t, T, F>
        {
            return tag_invoke(*this, std::forward<T>(binding), std::forward<F>(callback));
        }
    };

    constexpr access_binding_t access_binding {};


    template <typename B, typename T>
    concept Binding = requires(B &&binding) {
        {
            access_binding(binding, [](const T &) { })
        } -> std::same_as<bool>;
    };

    constexpr auto anyBindingCallback = [](const auto &) { };

    template <typename B>
    concept AnyBinding = requires(B &&binding) {
        {
            access_binding(binding, anyBindingCallback)
        } -> std::same_as<bool>;
    };


}
}