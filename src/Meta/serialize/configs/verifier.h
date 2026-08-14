#pragma once

#include "Generic/context.h"

#include "../context.h"
#include "../hierarchy/syncfunction.h"
#include "../streams/streamresult.h"
#include "configselector.h"

namespace Engine {
namespace Serialize {

    struct VerifierCategory;

    struct DefaultVerifier {
        using Category = VerifierCategory;

        template <typename Context, typename... Args>
        static bool verify(Context &&, Args &&...)
        {
            return true;
        }
    };
    template <auto f, typename R, typename T, typename... Args>
    struct CustomVerifierImpl {
        using Category = VerifierCategory;

        template <typename Context, typename... ExplicitArgs>
        static bool verify(Context &&context, ExplicitArgs... args)
        {
            bool verified = false;
            StreamResult result = context_invoke([&](auto &&...contextual) -> StreamResult {
                verified = TupleUnpacker::unpackTuple([&](auto &&...args) {
                    return f(std::forward<decltype(args)>(args)..., std::forward<decltype(contextual)>(contextual)...);
                },
                    std::forward_as_tuple(std::forward<ExplicitArgs>(args)...), make_index_pack<context_args<decltype(f)>::size> {});
                return {};
            },
                context_contextual<decltype(f)> {}, context);
            assert(!result.mError);
            return verified;
        }
    };

    template <auto f, typename R, typename T, typename... Args>
    struct ParentVerifierImpl {
        using Category = VerifierCategory;

        template <typename Context, typename... ExplicitArgs>
        static bool verify(Context &&context, ExplicitArgs... args)
        {
            bool verified = false;
            StreamResult result = context_invoke([&](T &parent, auto &&...contextual) -> StreamResult {
                verified = TupleUnpacker::unpackTuple([&](auto &&...args) {
                    return (parent.*f)(std::forward<decltype(args)>(args)..., std::forward<decltype(contextual)>(contextual)...);
                },
                    std::forward_as_tuple(std::forward<ExplicitArgs>(args)...), make_index_pack<context_args<decltype(f)>::size> {});                
                return {};
            },
                typename context_contextual<decltype(f)>::template prepend<Contextual<T>> {}, context);
            assert(!result.mError);
            return verified;
        }
    };

    template <auto f>
    using ParentVerifier = FunctionCapture<ParentVerifierImpl, f>;
    template <auto f>
    using CustomVerifier = FunctionCapture<CustomVerifierImpl, f>;

    template <typename... Configs>
    using VerifierSelector = ConfigSelectorDefault<VerifierCategory, DefaultVerifier, Configs...>;

}
}