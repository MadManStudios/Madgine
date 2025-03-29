#pragma once

#include "concepts.h"

namespace Engine {
namespace Execution {

    template <typename F, typename R, typename... V>
    struct SimpleSender {

        using result_type = R;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<V...>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(connect_t, SimpleSender &&sender, Rec &&rec)
        {
            return sender.mF(std::forward<Rec>(rec));
        }

        F mF;
    };

    template <typename R, typename... V, typename F>
    auto make_sender(F &&f)
    {
        return SimpleSender<F, R, V...> { std::forward<F>(f) };
    }

#define ASYNC_STUB(Name, Impl, ...)                                        \
    template <typename... Args>                                            \
    auto Name(Args &&...args)                                              \
    {                                                                      \
        return __VA_ARGS__(LIFT(Impl, this), std::forward<Args>(args)...); \
    }

}
}