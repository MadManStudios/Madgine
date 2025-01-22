#pragma once

#include "Generic/execution/sender.h"
#include "Generic/genericresult.h"

namespace Engine {
namespace FirstParty {

    template <typename R>
    R infoTypeHelper(void (*)(const R *));

    template <typename F>
    using InfoTypeHelper = decltype(infoTypeHelper(std::declval<F>()));

    template <auto f, typename R, typename Rec, typename... Args>
    struct eos_state {
        void start()
        {
            TupleUnpacker::invokeFromTuple([this](Args... args) {
                f(std::forward<Args>(args)..., this, &eos_state::callback);
            },
                std::move(mArgs));
        }
        static void callback(const R *result)
        {
            eos_state *self = static_cast<eos_state *>(result->ClientData);
            self->mRec.set_value(*result);
        }
        Rec mRec;
        std::tuple<Args...> mArgs;
    };

    template <auto f, typename... Args>
    auto EOS_sender(Args &&...args)
    {
        using argument_types = typename Callable<f>::traits::argument_types;
        using C = typename argument_types::template select<argument_types::size - 1>;

        using R = InfoTypeHelper<C>;
        return Execution::make_sender<GenericResult, R>(
            [args = std::tuple<Args...> { std::forward<Args>(args)... }]<typename Rec>(Rec &&rec) mutable {
                return eos_state<f, R, Rec, Args...> {
                    std::forward<Rec>(rec), std::move(args)
                };
            });
    }
}
}