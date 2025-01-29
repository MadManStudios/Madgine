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

    template <typename... Ty>
    struct make_virtual_sender_helper {
        template <typename F>
        static auto make(F&& f) {
            return make_sender<Ty...>(std::forward<F>(f));
        } 
    };

    template <typename State, typename... Args>
    auto make_virtual_sender(Args &&...args)
    {        
        return State::value_types::template prepend<typename State::result_types::first>::template instantiate<make_virtual_sender_helper>::make(
            [args = std::tuple<Args...> { std::forward<Args>(args)... }]<typename Rec>(Rec &&rec) mutable {
                return TupleUnpacker::constructExpand<VirtualState<Rec, State>>(std::forward<Rec>(rec), std::move(args));
            });
    }

    template <typename R, typename... V, typename F, typename... Args>
    auto make_simple_virtual_sender(F &&f, Args &&...args)
    {
        return make_virtual_sender<SimpleState<F, std::tuple<Args...>, VirtualReceiverBase<R, V...>>>(std::forward<F>(f), std::tuple<Args...> { std::forward<Args>(args)... });
    }

}
}