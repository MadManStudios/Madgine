#pragma once

#include "concepts.h"

namespace Engine {
namespace Execution {

    template <typename F, typename R, typename... V>
    struct SenderWrapper {

        using result_type = R;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<V...>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(connect_t, SenderWrapper &&sender, Rec &&rec)
        {
            return sender.mF(std::forward<Rec>(rec));
        }

        F mF;
    };

    template <typename R = void, typename... V, typename F>
    auto make_sender(F &&f)
    {
        return SenderWrapper<F, R, V...> { std::forward<F>(f) };
    }

#define ASYNC_STUB(Name, Impl, ...)                                        \
    template <typename... Args>                                            \
    auto Name(Args &&...args)                                              \
    {                                                                      \
        return __VA_ARGS__(LIFT(Impl, this), std::forward<Args>(args)...); \
    }

    template <typename F, typename Rec>
    struct SimpleState : base_state<Rec> {

        SimpleState(F &&f, Rec &&rec)
            : base_state<Rec>(std::forward<Rec>(rec))
            , mF(std::forward<F>(f))
        {
        }

        void start()
        {
            mF();
            this->set_value();
        }

        void stop()
        {
        }

        F mF;
    };

    template <typename F, typename Rec>
    auto make_simple_state(F &&f, Rec &&rec)
    {
        return SimpleState<F, Rec> { std::forward<F>(f), std::forward<Rec>(rec) };
    }

    template <typename R = void, typename F>
    auto make_simple_sender(F &&f)
    {
        return make_sender<R>([f { forward_capture<F>(f) }](auto &&rec) mutable { return make_simple_state(std::forward<F>(f), std::forward<decltype(rec)>(rec)); });
        ;
    }

}
}