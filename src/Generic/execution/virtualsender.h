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

    template <typename State, typename... Args>
    struct VirtualSender {

        VirtualSender(Args &&...args)
            : mArgs(std::forward<Args>(args)...)
        {
        }

        using result_type = State::result_types::first;
        template <template <typename...> typename Tuple>
        using value_types = typename State::value_types::template instantiate<Tuple>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(connect_t, VirtualSender &&sender, Rec &&rec)
        {
            return TupleUnpacker::constructExpand<VirtualState<Rec, State>>(std::forward<Rec>(rec), std::move(sender.mArgs));
        }

        std::tuple<Args...> mArgs;
    };

    
    template <typename State, typename... Args>
    auto make_virtual_sender(Args &&...args)
    {
        return VirtualSender<State, Args...>(std::forward<Args>(args)...);
    }

    template <typename R, typename... V, typename F, typename... Args>
    auto make_simple_virtual_sender(F &&f, Args &&...args)
    {
        return make_virtual_sender<SimpleState<F, std::tuple<Args...>, VirtualReceiverBase<R, V...>>>(std::forward<F>(f), std::tuple<Args...> { std::forward<Args>(args)... });
    }

}
}