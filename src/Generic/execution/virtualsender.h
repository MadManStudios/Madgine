#pragma once

#include "virtualstate.h"

namespace Engine {
namespace Execution {

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
            return TupleUnpacker::constructExpand<VirtualState<State, Rec>>(std::forward<Rec>(rec), std::move(sender.mArgs));
        }

        std::tuple<Args...> mArgs;
    };

    template <typename State, typename... Args>
    auto make_virtual_sender(Args &&...args)
    {
        return VirtualSender<State, Args...>(std::forward<Args>(args)...);
    }


}
}