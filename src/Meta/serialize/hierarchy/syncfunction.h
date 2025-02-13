#pragma once

namespace Engine {
namespace Serialize {

    struct SyncFunction {
        void (*mWriteFunctionArguments)(const std::vector<WriteMessage> &, const void *);
        void (*mWriteFunctionResult)(FormattedMessageStream &, const void *);
        StreamResult (*mReadFunctionAction)(SyncableUnitBase *, FormattedMessageStream &, uint16_t, FunctionType, PendingRequest &);
        StreamResult (*mReadFunctionRequest)(SyncableUnitBase *, FormattedMessageStream &, uint16_t, FunctionType, MessageId);
    };

    struct SyncFunctionContext {
        ParticipantId mCallerId;
    };

    template <typename Traits, typename args_pack>
    struct SyncFunctionTraitsHelper : Traits {
        static decltype(auto) patchArgs(auto &&args, SyncFunctionContext context)
        {
            return std::forward<decltype(args)>(args);
        }
    };

    template <typename Traits, typename... Args>
    struct SyncFunctionTraitsHelper<Traits, type_pack<SyncFunctionContext, Args...>> : Traits {
        using decay_argument_types = type_pack<Args...>;

        static decltype(auto) patchArgs(auto &&args, SyncFunctionContext context)
        {
            return TupleUnpacker::prepend(std::move(context), std::forward<decltype(args)>(args));
        }
    };

    template <typename Traits>
    using SyncFunctionTraits = SyncFunctionTraitsHelper<Traits, typename Traits::decay_argument_types>;

}
}