#pragma once

#include "Generic/linestruct.h"
#include "Generic/memberoffsetptr.h"

#include "configs/verifier.h"
#include "container/container_operations.h"
#include "hierarchy/serializer.h"
#include "hierarchy/serializetable.h"
#include "hierarchy/syncfunction.h"
#include "operations.h"

namespace Engine {
namespace Serialize {

    META_EXPORT void writeFunctionAction(SyncableUnitBase *unit, uint16_t index, const void *args, const std::set<ParticipantId> &targets, ParticipantId answerTarget, MessageId answerId);
    META_EXPORT void writeFunctionResult(SyncableUnitBase *unit, uint16_t index, const void *result, FormattedMessageStream &target, MessageId answerId);
    META_EXPORT void writeFunctionRequest(SyncableUnitBase *unit, uint16_t index, FunctionType type, const void *args, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver = {});
    META_EXPORT void writeFunctionError(SyncableUnitBase *unit, uint16_t index, MessageResult error, FormattedMessageStream &target, MessageId answerId);
    META_EXPORT StreamResult readState(const SerializeTable *table, void *unit, FormattedSerializeStream &in, ContextPtr context);
    META_EXPORT WriteMessage getMasterRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId = 0);
    META_EXPORT FormattedMessageStream &getMasterFunctionRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget);

    namespace __serialize_impl__ {

        struct SerializerTag;
        struct FunctionTag;

        template <typename _disambiguate__dont_remove, auto P, auto Getter, auto Setter>
        constexpr Serializer encapsulated_pointer(const char *name)
        {
            using traits = CallableTraits<decltype(P)>;
            using Unit = typename traits::class_type;
            using T = std::decay_t<typename traits::return_type>;

            using getter_traits = CallableTraits<decltype(Getter)>;
            static_assert(std::same_as<Unit, std::remove_const_t<typename getter_traits::class_type>>);
            static_assert(std::same_as<T, typename getter_traits::return_type>);

            using setter_traits = CallableTraits<decltype(Setter)>;
            static_assert(std::same_as<Unit, typename setter_traits::class_type>);

            // TODO remove const in tuple types
            static_assert(std::same_as<typename setter_traits::argument_types, type_pack<T>>);

            return {
                name,
                OffsetPtr {},
                [](const void *_unit, FormattedSerializeStream &out, const char *name, ContextPtr context) {
                    const Unit *unit = unit_cast<const Unit *>(_unit);
                    write(out, (unit->*Getter)(), name, context_set(context, *unit));
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, ContextPtr context) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    (unit->*Setter)(nullptr);
                    return read(in, unit->*P, name, context_set(context, *unit));
                },
                [](SyncableUnitBase *unit, FormattedMessageStream &in, PendingRequest *request) -> StreamResult {
                    throw "Unsupported";
                },
                [](SyncableUnitBase *unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    throw "Unsupported";
                },
                [](void *_unit, FormattedSerializeStream &in, bool success) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    return apply_map(unit->*P, in, success);
                },
                [](void *unit, bool b) {
                },
                [](void *_unit, bool active, bool existenceChanged) {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    if (active) {
                        T val = unit->*P;
                        unit->*P = nullptr;
                        (unit->*Setter)(val);
                    } else {
                        T val = unit->*P;
                        (unit->*Setter)(nullptr);
                        unit->*P = val;
                    }
                },
                [](void *unit) {
                },
                [](const SyncableUnitBase *unit, const std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> &outStreams, void *data) {
                    throw "Unsupported";
                },
                [](const SyncableUnitBase *_unit, FormattedMessageStream &out, void *data) {
                    throw "Unsupported";
                },
                [](size_t, FormattedSerializeStream &, const char *, const StreamVisitor &) -> StreamResult {
                    return {};
                }
            };
        }

        template <typename _disambiguate__dont_remove, auto Getter, auto Setter, typename... Configs>
        constexpr Serializer encapsulated_field(const char *name)
        {

            using getter_traits = CallableTraits<decltype(Getter)>;
            using getter_unit = std::decay_t<typename getter_traits::class_type>;
            using T = std::decay_t<typename getter_traits::return_type>;

            using setter_traits = CallableTraits<decltype(Setter)>;
            using setter_unit = std::decay_t<typename setter_traits::class_type>;

            return {
                name,
                OffsetPtr {},
                [](const void *_unit, FormattedSerializeStream &out, const char *name, ContextPtr context) {
                    const getter_unit *unit = unit_cast<const getter_unit>(_unit);
                    write<T, Configs...>(out, (unit->*Getter)(), name, context_set(context, *unit));
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, ContextPtr context) -> StreamResult {
                    setter_unit *unit = unit_cast<setter_unit>(_unit);
                    MakeOwning_t<T> dummy;
                    STREAM_PROPAGATE_ERROR(read<MakeOwning_t<T>, Configs...>(in, dummy, name, context_set(context, *unit)));

                    return context_invoke([&](auto &&...args) -> StreamResult {
                        std::invoke(Setter, unit, std::move(dummy), std::forward<decltype(args)>(args)...);
                        return {};
                    },
                        typename setter_traits::argument_types::pop_front {}, context);
                },
                [](void *unit, FormattedSerializeStream &in, PendingRequest &request, ContextPtr context) -> StreamResult {
                    throw "Unsupported";
                },
                [](void *unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    throw "Unsupported";
                },
                [](const Serializer *, void *_unit, FormattedSerializeStream &in, bool success, ContextPtr context) {
                    return StreamResult {};
                },
                [](const Serialize::Serializer *serializer, void *unit, bool b, ContextPtr context) {
                },
                [](const Serialize::Serializer *serializer, void *unit, bool active, bool existenceChanged, ContextPtr context) {
                },
                [](const Serialize::Serializer *serializer, void *unit) {
                },
                [](const void *unit, const std::vector<WriteMessage> &outStreams, void *data) {
                    throw "Unsupported";
                },
                [](const void *_unit, FormattedSerializeStream &out, void *data, ContextPtr context) {
                    throw "Unsupported";
                },
                [](FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth) -> StreamResult {
                    return Serialize::visitStream<T, Configs...>(in, name, visitor, depth);
                }
            };
        }

        template <typename _disambiguate__dont_remove, uintptr_t off, auto P, typename... Configs, typename... ParentConfigs>
        constexpr Serializer field(const char *name, type_pack<ParentConfigs...>)
        {
            using traits = CallableTraits<decltype(P)>;
            using Unit = typename traits::class_type;
            using R = typename traits::return_type;
            using T = std::decay_t<R>;

            return {
                name,
                OffsetPtr { off },
                [](const void *_unit, FormattedSerializeStream &out, const char *name, ContextPtr context) {
                    const Unit *unit = unit_cast<const Unit>(_unit);
                    write<T, Configs...>(out, std::invoke(P, unit), name, context_set(context, *unit));
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, ContextPtr context) -> StreamResult {
                    Unit *unit = unit_cast<Unit>(_unit);
                    return read<T, Configs...>(in, unit->*P, name, context_set(context, *unit));
                },
                [](void *_unit, FormattedSerializeStream &in, PendingRequest &request, ContextPtr context) -> StreamResult {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        Unit *unit = unit_cast<Unit>(_unit);
                        return readAction<T, ParentConfigs..., Configs...>(unit->*P, in, request, context_set(context, *unit));
                    } else
                        throw "Unsupported";
                },
                [](void *_unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        Unit *unit = unit_cast<Unit>(_unit);
                        return readRequest<T, ParentConfigs..., Configs...>(unit->*P, inout, id, context_set(ContextPtr {}, *unit));
                    } else
                        throw "Unsupported";
                },
                [](const Serializer *, void *_unit, FormattedSerializeStream &in, bool success, ContextPtr context) -> StreamResult {
                    Unit *unit = unit_cast<Unit>(_unit);
                    return apply_map(unit->*P, in, success, context_set(context, *unit));
                },
                [](const Serializer *, void *_unit, bool b, ContextPtr context) {
                    Unit *unit = unit_cast<Unit>(_unit);
                    set_synced(unit->*P, b, context_set(context, *unit));
                },
                [](const Serializer *, void *unit, bool active, bool existenceChanged, ContextPtr context) {
                    set_active<Configs...>(unit_cast<Unit>(unit)->*P, active, existenceChanged, context);
                },
                [](const Serializer *, void *unit) {
                    set_parent(unit_cast<Unit>(unit)->*P, unit_cast<Unit>(unit));
                },
                [](const void *_unit, const std::vector<WriteMessage> &outStreams, void *data) {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        const Unit *unit = unit_cast<const Unit>(_unit);
                        typename T::action_payload &payload = *static_cast<typename T::action_payload *>(data);
                        writeAction<T, Configs...>(unit->*P, outStreams, std::move(payload), context_set(ContextPtr {}, *unit));
                    } else
                        throw "Unsupported";
                },
                [](const void *_unit, FormattedSerializeStream &out, void *data, ContextPtr context) {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        const Unit *unit = unit_cast<const Unit>(_unit);
                        typename T::request_payload &payload = *static_cast<typename T::request_payload *>(data);
                        writeRequest<T, Configs...>(unit->*P, out, std::move(payload), context_set(context, *unit));
                    } else
                        throw "Unsupported";
                },
                [](FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth) -> StreamResult {
                    return visitStream<T, Configs...>(in, name, visitor, depth);
                }
            };
        }

        template <auto f, typename... Configs>
        constexpr SyncFunction syncFunction()
        {
            using traits = typename Callable<f>::traits;
            using R = patch_void_t<typename traits::return_type, std::monostate>;
            using T = typename traits::class_type;
            using Tuple = typename context_args<decltype(f)>::as_tuple;
            using OwningTuple = typename context_args<decltype(f)>::template transform<MakeOwning_t>::as_tuple;

            return {
                [](const std::vector<WriteMessage> &outStreams, const void *args) {
                    const Tuple &argTuple = *static_cast<const Tuple *>(args);
                    for (FormattedMessageStream &out : outStreams) {
                        write(out, argTuple, "Args");
                    }
                },
                [](FormattedSerializeStream &out, const void *result) {
                    write(out, *static_cast<const R *>(result), "Result");
                },
                [](SyncableUnitBase *unit, FormattedSerializeStream &in, uint16_t index, FunctionType type, PendingRequest &request) {
                    switch (type) {
                    case CALL: {
                        OwningTuple owningArgs;
                        STREAM_PROPAGATE_ERROR(read(in, owningArgs, "Args"));
                        STREAM_PROPAGATE_ERROR(apply_map(owningArgs, in, true));
                        Tuple args = owningArgs;
                        writeFunctionAction(unit, index, &args, {}, request.mRequester, request.mRequesterTransactionId);
                        R result;

                        STREAM_PROPAGATE_ERROR(context_invoke([&](auto &&...contextArgs) -> StreamResult {
                            result = TupleUnpacker::invokeFromTuple([&](auto &&...args) {
                                return patch_void(f, std::monostate {})(static_cast<T *>(unit), std::forward<decltype(args)>(args)..., std::forward<decltype(contextArgs)>(contextArgs)...);
                            },
                                std::move(args));
                            return {};
                        },
                            context_contextual<decltype(f)> {}, SyncFunctionContext { in.id() }));

                        request.mReceiver.set_value<R>(result);
                    } break;
                    case QUERY: {
                        R result;
                        STREAM_PROPAGATE_ERROR(read(in, result, "Result"));
                        if (request.mRequesterTransactionId) {
                            FormattedMessageStream &out = getMasterFunctionRequestResponseTarget(unit, request.mRequester);
                            writeFunctionResult(unit, index, &result, out, request.mRequesterTransactionId);
                        }
                        request.mReceiver.set_value<R>(result);
                    } break;
                    }
                    return StreamResult {};
                },
                [](SyncableUnitBase *_unit, FormattedMessageStream &in, uint16_t index, FunctionType type, MessageId id) {
                    T *unit = static_cast<T *>(_unit);
                    OwningTuple owningArgs;
                    STREAM_PROPAGATE_ERROR(read(in, owningArgs, "Args"));
                    STREAM_PROPAGATE_ERROR(apply_map(owningArgs, in, true));
                    ParticipantId answerId = in.id();
                    Tuple args = owningArgs;
                    auto &&context = context_set(SyncFunctionContext { answerId }, *unit);
                    if (!TupleUnpacker::invokeFromTuple([&](auto &&...args) { return VerifierSelector<Configs...>::verify(context, std::forward<decltype(args)>(args)...); }, args)) {
                        writeFunctionError(unit, index, MessageResult::REJECTED, in, id);
                    } else if (unit->isMaster()) {
                        if (type == CALL)
                            writeFunctionAction(unit, index, &args, {}, answerId, id);

                        R result;
                        STREAM_PROPAGATE_ERROR(context_invoke([&](auto &&...contextArgs) -> StreamResult {
                            result = TupleUnpacker::invokeFromTuple([&](auto &&...args) {
                                return patch_void(f, std::monostate {})(static_cast<T *>(unit), std::forward<decltype(args)>(args)..., std::forward<decltype(contextArgs)>(contextArgs)...);
                            },
                                std::move(args));
                            return {};
                        },
                            context_contextual<decltype(f)> {}, context));
                        
                        if (type == QUERY && id != 0)
                            writeFunctionResult(unit, index, &result, in, id);
                    } else {
                        writeFunctionRequest(
                            unit, index, type, &args, answerId, id);
                    }
                    return StreamResult {};
                }
            };
        }

        template <typename T, typename... Configs>
        StreamResult readState(const SerializeTable *table, void *unit, FormattedSerializeStream &in, ContextPtr context)
        {
            auto &&newContext = context_set(context, *unit_cast<T>(unit));

            auto guard = GuardSelector<Configs...>::guard(newContext);
            (void)guard;
            return Serialize::readState(table, unit, in, newContext);
        }
    }

}
}

#define SERIALIZETABLE_BEGIN_IMPL_EX(Idx, T, Base, ...)                                                                           \
    namespace Serialize_##T                                                                                                       \
    {                                                                                                                             \
        static constexpr const ::Engine::Serialize::SerializeTable &(*baseType)() = Base;                                         \
        static constexpr const auto readState = ::Engine::Serialize::__serialize_impl__::readState<T __VA_OPT__(, ) __VA_ARGS__>; \
    }                                                                                                                             \
    namespace Engine {                                                                                                            \
        START_STRUCT(Serialize::__serialize_impl__::SerializerTag, Idx)                                                           \
        {                                                                                                                         \
            using Ty = T;                                                                                                         \
            static constexpr auto parentConfigs = type_pack<__VA_ARGS__> {};                                                      \
            static constexpr const bool base = true;                                                                              \
            constexpr const Serialize::Serializer *data() const;                                                                  \
        };                                                                                                                        \
        START_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)                                                             \
        {                                                                                                                         \
            using Ty = T;                                                                                                         \
            static constexpr const bool base = true;                                                                              \
            static constexpr const size_t count = 0;                                                                              \
            constexpr const Serialize::SyncFunction *data() const { return nullptr; }                                             \
            template <auto g>                                                                                                     \
            static constexpr uint16_t getIndex();                                                                                 \
        };                                                                                                                        \
    }

#define SERIALIZETABLE_INHERIT_BEGIN_EX(Idx, T, Base, ...) SERIALIZETABLE_BEGIN_IMPL_EX(Idx, T, &serializeTable<Base>, __VA_ARGS__)
#define SERIALIZETABLE_BEGIN_EX(Idx, T, ...) SERIALIZETABLE_BEGIN_IMPL_EX(Idx, T, nullptr, __VA_ARGS__)

#define SERIALIZETABLE_INHERIT_BEGIN(T, Base, ...) SERIALIZETABLE_INHERIT_BEGIN_EX(, T, Base, __VA_ARGS__)
#define SERIALIZETABLE_BEGIN(T, ...) SERIALIZETABLE_BEGIN_EX(, T, __VA_ARGS__)

#define SERIALIZETABLE_ENTRY_EX(Idx, Ser)                                                           \
    namespace Engine {                                                                              \
        LINE_STRUCT(Serialize::__serialize_impl__::SerializerTag, Idx)                              \
        {                                                                                           \
            constexpr const Serialize::Serializer *data() const                                     \
            {                                                                                       \
                if constexpr (BASE_STRUCT(Serialize::__serialize_impl__::SerializerTag, Idx)::base) \
                    return &mData;                                                                  \
                else                                                                                \
                    return BASE_STRUCT(Serialize::__serialize_impl__::SerializerTag, Idx)::data();  \
            }                                                                                       \
            static constexpr const bool base = false;                                               \
            Serialize::Serializer mData = Ser;                                                      \
        };                                                                                          \
    }

#define SERIALIZETABLE_ENTRY(Ser) \
    SERIALIZETABLE_ENTRY_EX(, Ser)

#define SYNCFUNCTION_EX(Idx, f, ...)                                                                                          \
    namespace Engine {                                                                                                        \
        LINE_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)                                                          \
        {                                                                                                                     \
            constexpr const Serialize::SyncFunction *data() const                                                             \
            {                                                                                                                 \
                if constexpr (BASE_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)::base)                             \
                    return &mData;                                                                                            \
                else                                                                                                          \
                    return BASE_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)::data();                              \
            }                                                                                                                 \
            static constexpr const bool base = false;                                                                         \
            static constexpr const size_t count = BASE_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)::count + 1;    \
            Serialize::SyncFunction mData = Serialize::__serialize_impl__::syncFunction<&Ty::f __VA_OPT__(, ) __VA_ARGS__>(); \
            template <auto g>                                                                                                 \
            static constexpr uint16_t getIndex()                                                                              \
            {                                                                                                                 \
                if constexpr (FSameAs<&Ty::f, g>)                                                                             \
                    return count - 1;                                                                                         \
                else                                                                                                          \
                    return BASE_STRUCT(Serialize::__serialize_impl__::FunctionTag, Idx)::getIndex<g>();                       \
            }                                                                                                                 \
        };                                                                                                                    \
    }

#define SYNCFUNCTION(f, ...) \
    SYNCFUNCTION_EX(, f, __VA_ARGS__)

#define SERIALIZETABLE_END_EX(Idx, T)                                                                                 \
    SERIALIZETABLE_ENTRY_EX(Idx, { nullptr })                                                                         \
    namespace Serialize_##T                                                                                           \
    {                                                                                                                 \
        static constexpr GET_STRUCT(::Engine::Serialize::__serialize_impl__::SerializerTag, Idx) fields = {};         \
        static constexpr GET_STRUCT(::Engine::Serialize::__serialize_impl__::FunctionTag, Idx) functions = {};        \
    };                                                                                                                \
    namespace Engine {                                                                                                \
        namespace Serialize {                                                                                         \
            namespace __serialize_impl__ {                                                                            \
                template <>                                                                                           \
                struct SyncFunctionTable<T> : GET_STRUCT(::Engine::Serialize::__serialize_impl__::FunctionTag, Idx) { \
                };                                                                                                    \
            }                                                                                                         \
        }                                                                                                             \
    }                                                                                                                 \
    DLL_EXPORT_VARIABLE2(constexpr, const ::Engine::Serialize::SerializeTable, ::, serializeTable, SINGLE_ARG({ #T, ::Engine::type_holder<T>, ::Serialize_##T::baseType, ::Serialize_##T::readState, ::Serialize_##T::fields.data(), ::Serialize_##T::functions.data(), std::derived_from<T, ::Engine::Serialize::TopLevelUnitBase> }), T);

#define SERIALIZETABLE_END(T) \
    SERIALIZETABLE_END_EX     \
    (, T)

#define FIELD_EX(Idx, ...) \
    SERIALIZETABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Serialize::__serialize_impl__::field<Ty, offsetof(Ty, FIRST(__VA_ARGS__)), &Ty::__VA_ARGS__>(STRINGIFY2(FIRST(__VA_ARGS__)), parentConfigs)))

#define FIELD(...) \
    FIELD_EX(, __VA_ARGS__)

#define ENCAPSULATED_FIELD_EX(Idx, Name, Getter /*, Setter*/, ...) \
    SERIALIZETABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Serialize::__serialize_impl__::encapsulated_field<Ty, &Ty::Getter, &Ty::__VA_ARGS__>(#Name)))

#define ENCAPSULATED_FIELD(Name, Getter /*, Setter*/, ...) \
    ENCAPSULATED_FIELD_EX(, Name, Getter, __VA_ARGS__)

#define ENCAPSULATED_POINTER_EX(Idx, Name, Getter, Setter) \
    SERIALIZETABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Serialize::__serialize_impl__::encapsulated_pointer<Ty, &Ty::Name, &Ty::Getter, &Ty::Setter>(#Name)))

#define ENCAPSULATED_POINTER(Name, Getter, Setter) \
    ENCAPSULATED_POINTER_EX(, Name, Getter, Setter)
