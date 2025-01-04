#pragma once

#include "sender.h"
#include "virtualstate.h"

namespace Engine {
namespace Execution {

    template <typename F, typename Tuple, typename State>
    struct SimpleState : State {

        template <typename... Args>
        SimpleState(F &&f, Tuple &&args, Args &&...baseArgs)
            : State(std::forward<Args>(baseArgs)...)
            , mF(std::forward<F>(f))
            , mArgs(std::move(args))
        {
        }

        void start()
        {
            TupleUnpacker::invokeExpand(std::forward<F>(mF), *this, std::move(mArgs));
        }

        F mF;
        Tuple mArgs;
    };

    template <typename State, typename F, typename Tuple, typename... Args>
    auto make_simple_state(F &&f, Tuple &&args, Args &&...baseArgs)
    {
        return SimpleState<F, Tuple, State> { std::forward<F>(f), std::forward<Tuple>(args), std::forward<Args>(baseArgs)... };
    }

    template <typename State, typename R, typename... V, typename Rec, typename... Args>
    auto make_virtual_state(Rec &&rec, Args &&...args)
    {
        return VirtualStateEx<Rec, State, make_type_pack_t<R>, type_pack<V...>> { std::forward<Rec>(rec), std::forward<Args>(args)... };
    }

    template <typename State, typename R, typename... V, typename... Args>
    auto make_virtual_sender(Args &&...args)
    {
        return make_sender<R, V...>(
            [args = std::tuple<Args...> { std::forward<Args>(args)... }]<typename Rec>(Rec &&rec) mutable {
                return TupleUnpacker::invokeExpand(LIFT(SINGLE_ARG(make_virtual_state<State, R, V...>)), std::forward<Rec>(rec), std::move(args));
            });
    }

    template <typename R, typename... V, typename F, typename... Args>
    auto make_simple_virtual_sender(F &&f, Args &&...args)
    {
        return make_virtual_sender<SimpleState<F, std::tuple<Args...>, VirtualReceiverBase<R, V...>>, R, V...>(std::forward<F>(f), std::tuple<Args...> { std::forward<Args>(args)... });
    }

}
}