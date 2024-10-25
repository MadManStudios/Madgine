#pragma once

#include "../tag_invoke.h"

#include "../fixed_string.h"

namespace Engine {
namespace Execution {

    template <typename T>
    concept Sender = requires
    {
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
        template <typename Sender, typename Rec>
        requires(!tag_invocable<outer_connect_t, Sender, Rec>) auto operator()(Sender &&sender, Rec &&rec) const
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

    struct get_receiver_t {
        template <typename T>
        requires tag_invocable<get_receiver_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_receiver_t, T &>)
                -> tag_invoke_result_t<get_receiver_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_receiver_t get_receiver;

    struct unstoppable_token {
        operator std::stop_token()
        {
            return {};
        }

        static constexpr bool stop_requested()
        {
            return false;
        }
    };

    struct get_stop_token_t {

        template <typename T>
        requires(!tag_invocable<get_stop_token_t, T &>) auto operator()(T &) const
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
        using inner = Rec;

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
        requires(is_tag_invocable_v<CPO, Rec &, Args...>) friend auto tag_invoke(CPO f, algorithm_receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, Rec &, Args...>)
            -> tag_invoke_result_t<CPO, Rec &, Args...>
        {
            return tag_invoke(f, rec.mRec, std::forward<Args>(args)...);
        }
    };

    template <typename _Rec>
    struct base_state {
        using Rec = _Rec;

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

        friend Rec &tag_invoke(get_receiver_t, base_state &state)
        {
            return state.mRec;
        }

        friend const Rec &tag_invoke(get_receiver_t, const base_state &state)
        {
            return state.mRec;
        }
    };

    template <typename State>
    struct algorithm_state_helper {

        using InnerRec = typename State::Rec;
        using Rec = typename InnerRec::inner;

        template <typename Sender, typename... Args>
        algorithm_state_helper(Sender &&sender, Rec &&rec, Args &&...args)
            : mState { connect(std::forward<Sender>(sender), InnerRec { std::forward<Rec>(rec), std::forward<Args>(args)... }) }
        {
        }

        ~algorithm_state_helper() { }

        void start()
        {
            mState.start();
        }

        template <typename... V>
        void set_value(Rec &rec, V &&...value)
        {
            rec.set_value(std::forward<V>(value)...);
        }

        void set_done(Rec &rec)
        {
            rec.set_done();
        }

        template <typename... R>
        void set_error(Rec &rec, R &&...result)
        {
            rec.set_error(std::forward<R>(result)...);
        }

        State mState;

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, algorithm_state_helper &state, Args &&...args)
            -> tag_invoke_result_t<CPO, State &, Args...>
        {
            return f(state.mState, std::forward<Args>(args)...);
        }

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, const algorithm_state_helper &state, Args &&...args)
            -> tag_invoke_result_t<CPO, const State &, Args...>
        {
            return f(state.mState, std::forward<Args>(args)...);
        }
    };

    template <typename Sender, typename InnerRec>
    using algorithm_state = algorithm_state_helper<connect_result_t<Sender, InnerRec>>;

    struct base_sender {
        using is_sender = void;
    };

    template <typename Sender>
    struct algorithm_sender : base_sender {
        using result_type = typename std::decay_t<Sender>::result_type;
        template <template <typename...> typename Tuple>
        using value_types = typename std::decay_t<Sender>::template value_types<Tuple>;

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, algorithm_sender &sender, Args &&...args)
            -> tag_invoke_result_t<CPO, Sender &, Args...>
        {
            return f(sender.mSender, std::forward<Args>(args)...);
        }
        /*
        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, const algorithm_sender &sender, Args &&...args)
            -> tag_invoke_result_t<CPO, const Sender &, Args...>
        {
            return f(sender.mSender, std::forward<Args>(args)...);
        }*/

        Sender mSender;
    };

    template <typename... T>
    struct signature : type_pack<T...> {
    };

    template <typename T>
    struct stream;
    
    template <typename T>
    using is_stream = is_instance<T, stream>;

    template <fixed_string Name>
    struct variable_name_tag;

    template <fixed_string Name>
    using variable_type = TaggedPlaceholder<variable_name_tag<Name>, 0>;

    template <size_t I>
    struct dynamic_argument_type {
        static constexpr size_t index = I;
    };

    template <fixed_string Name, typename T>
    struct variable;


}
}