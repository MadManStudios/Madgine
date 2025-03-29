#pragma once

#include "hierarchy/serializer.h"
#include "hierarchy/syncfunction.h"

#include "Generic/memberoffsetptr.h"

#include "hierarchy/serializetable.h"

#include "operations.h"

#include "container/container_operations.h"

#include "configs/verifier.h"

#include "Generic/linestruct.h"

namespace Engine {
namespace Serialize {

    void META_EXPORT writeFunctionAction(SyncableUnitBase *unit, uint16_t index, const void *args, const std::set<ParticipantId> &targets, ParticipantId answerTarget, MessageId answerId);
    void META_EXPORT writeFunctionResult(SyncableUnitBase *unit, uint16_t index, const void *result, FormattedMessageStream &target, MessageId answerId);
    void META_EXPORT writeFunctionRequest(SyncableUnitBase *unit, uint16_t index, FunctionType type, const void *args, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver = {});
    void META_EXPORT writeFunctionError(SyncableUnitBase *unit, uint16_t index, MessageResult error, FormattedMessageStream &target, MessageId answerId);
    StreamResult META_EXPORT readState(const SerializeTable *table, void *unit, FormattedSerializeStream &in, CallerHierarchyBasePtr hierarchy);

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

            //TODO remove const in tuple types
            static_assert(std::same_as<typename setter_traits::argument_types, type_pack<T>>);

            return {
                name,
                []() {
                    return OffsetPtr {};
                },
                [](const void *_unit, FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) {
                    const Unit *unit = unit_cast<const Unit *>(_unit);
                    write(out, (unit->*Getter)(), name, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    (unit->*Setter)(nullptr);
                    return read(in, unit->*P, name, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](SyncableUnitBase *unit, FormattedMessageStream &in, PendingRequest *request) -> StreamResult {
                    throw "Unsupported";
                },
                [](SyncableUnitBase *unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    throw "Unsupported";
                },
                [](void *_unit, FormattedSerializeStream &in, bool success) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    STREAM_PROPAGATE_ERROR(apply_map(unit->*P, in, success));
                    return {};
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

            static_assert(requires(setter_unit * unit, MakeOwning_t<T> dummy, CallerHierarchyBasePtr hierarchy) {
                TupleUnpacker::invoke(Setter, unit, std::move(dummy), hierarchy);
            });

            return {
                name,
                []() {
                    return OffsetPtr {};
                },
                [](const void *_unit, FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) {
                    const getter_unit *unit = unit_cast<const getter_unit *>(_unit);
                    write<T, Configs...>(out, (unit->*Getter)(), name, hierarchy.append(unit));
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy) -> StreamResult {
                    setter_unit *unit = unit_cast<setter_unit *>(_unit);
                    MakeOwning_t<T> dummy;
                    STREAM_PROPAGATE_ERROR(read<MakeOwning_t<T>, Configs...>(in, dummy, name, CallerHierarchyPtr { hierarchy.append(unit) }));
                    TupleUnpacker::invoke(Setter, unit, std::move(dummy), hierarchy);
                    return {};
                },
                [](void *unit, FormattedMessageStream &in, PendingRequest &request) -> StreamResult {
                    throw "Unsupported";
                },
                [](void *unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    throw "Unsupported";
                },
                [](void *_unit, FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy) {
                    return StreamResult {};
                },
                [](void *unit, bool b, const CallerHierarchyBasePtr &hierarchy) {
                },
                [](void *unit, bool active, bool existenceChanged) {
                },
                [](void *unit) {
                },
                [](const void *unit, const std::vector<WriteMessage> &outStreams, void *data) {
                    throw "Unsupported";
                },
                [](const void *_unit, FormattedMessageStream &out, void *data) {
                    throw "Unsupported";
                },
                [](FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor) -> StreamResult {
                    return Serialize::visitStream<T, Configs...>(in, name, visitor);
                }
            };
        }

        template <typename _disambiguate__dont_remove, auto P, typename... Configs, typename... ParentConfigs>
        constexpr Serializer field(const char *name, type_pack<ParentConfigs...>)
        {
            using traits = CallableTraits<decltype(P)>;
            using Unit = typename traits::class_type;
            using T = std::decay_t<typename traits::return_type>;

            return {
                name,
                []() {
                    return MemberOffsetPtr<Unit, T> { P }.mOffset;
                },
                [](const void *_unit, FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy) {
                    const Unit *unit = unit_cast<const Unit *>(_unit);
                    write<T, Configs...>(out, std::invoke(P, unit), name, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](void *_unit, FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    return read<T, Configs...>(in, unit->*P, name, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](void *_unit, FormattedMessageStream &in, PendingRequest &request) -> StreamResult {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        Unit *unit = unit_cast<Unit *>(_unit);
                        return readAction<T, ParentConfigs..., Configs...>(unit->*P, in, request, CallerHierarchyPtr { CallerHierarchy { unit } });
                    } else
                        throw "Unsupported";
                },
                [](void *_unit, FormattedMessageStream &inout, MessageId id) -> StreamResult {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        Unit *unit = unit_cast<Unit *>(_unit);
                        return readRequest<T, ParentConfigs..., Configs...>(unit->*P, inout, id, CallerHierarchyPtr { CallerHierarchy { unit } });
                    } else
                        throw "Unsupported";
                },
                [](void *_unit, FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy) -> StreamResult {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    return apply_map(unit->*P, in, success, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](void *_unit, bool b, const CallerHierarchyBasePtr &hierarchy) {
                    Unit *unit = unit_cast<Unit *>(_unit);
                    set_synced(unit->*P, b, CallerHierarchyPtr { hierarchy.append(unit) });
                },
                [](void *unit, bool active, bool existenceChanged) {
                    setActive<T, Configs...>(unit_cast<Unit *>(unit)->*P, active, existenceChanged);
                },
                [](void *unit) {
                    set_parent(unit_cast<Unit *>(unit)->*P, unit_cast<Unit*>(unit));
                },
                [](const void *_unit, const std::vector<WriteMessage> &outStreams, void *data) {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        const Unit *unit = unit_cast<const Unit *>(_unit);
                        typename T::action_payload &payload = *static_cast<typename T::action_payload *>(data);
                        writeAction<T, Configs...>(unit->*P, outStreams, std::move(payload), CallerHierarchyPtr { CallerHierarchy { unit } });
                    } else
                        throw "Unsupported";
                },
                [](const void *_unit, FormattedMessageStream &out, void *data) {
                    if constexpr (std::derived_from<T, SyncableBase>) {
                        const Unit *unit = unit_cast<const Unit *>(_unit);
                        typename T::request_payload &payload = *static_cast<typename T::request_payload *>(data);
                        writeRequest<T, Configs...>(unit->*P, out, std::move(payload), CallerHierarchyPtr { CallerHierarchy { unit } });
                    } else
                        throw "Unsupported";
                },
                [](FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor) -> StreamResult {
                    return visitStream<T, Configs...>(in, name, visitor);
                }
            };
        }

        template <auto f, typename... Configs>
        constexpr SyncFunction syncFunction()
        {
            using traits = SyncFunctionTraits<typename Callable<f>::traits>;
            using R = patch_void_t<typename traits::return_type>;
            using T = typename traits::class_type;
            using Tuple = typename traits::decay_argument_types::as_tuple;

            return {
                [](const std::vector<WriteMessage> &outStreams, const void *args) {
                    const Tuple &argTuple = *static_cast<const Tuple *>(args);
                    for (FormattedMessageStream &out : outStreams) {
                        write(out, argTuple, "Args");
                    }
                },
                [](FormattedMessageStream &out, const void *result) {
                    write(out, *static_cast<const R *>(result), "Result");
                },
                [](SyncableUnitBase *unit, FormattedMessageStream &in, uint16_t index, FunctionType type, PendingRequest &request) {
                    switch (type) {
                    case CALL: {
                        Tuple args;
                        STREAM_PROPAGATE_ERROR(read(in, args, "Args"));
                        STREAM_PROPAGATE_ERROR(apply_map(args, in, true));
                        writeFunctionAction(unit, index, &args, {}, request.mRequester, request.mRequesterTransactionId);
                        R result = invoke_patch_void(LIFT(TupleUnpacker::invokeExpand), f, static_cast<T *>(unit), traits::patchArgs(std::move(args), { in.id() }));
                        request.mReceiver.set_value(result);
                    } break;
                    case QUERY: {
                        R result;
                        STREAM_PROPAGATE_ERROR(read(in, result, "Result"));
                        if (request.mRequesterTransactionId) {
                            FormattedMessageStream &out = getMasterRequestResponseTarget(unit, request.mRequester);
                            writeFunctionResult(unit, index, &result, out, request.mRequesterTransactionId);
                        }
                        request.mReceiver.set_value(result);
                    } break;
                    }
                    return StreamResult {};
                },
                [](SyncableUnitBase *_unit, FormattedMessageStream &in, uint16_t index, FunctionType type, MessageId id) {
                    T *unit = static_cast<T *>(_unit);
                    Tuple args;
                    STREAM_PROPAGATE_ERROR(read(in, args, "Args"));
                    STREAM_PROPAGATE_ERROR(apply_map(args, in, true));
                    ParticipantId answerId = in.id();
                    SyncFunctionContext context { answerId };
                    if (!TupleUnpacker::invokeExpand(VerifierSelector<Configs...>::verify, CallerHierarchyPtr { CallerHierarchy { unit } }, context, args)) {
                        writeFunctionError(unit, index, MessageResult::REJECTED, in, id);
                    } else if (unit->isMaster()) {
                        if (type == CALL)
                            writeFunctionAction(unit, index, &args, {}, answerId, id);
                        R result = invoke_patch_void(LIFT(TupleUnpacker::invokeExpand), f, unit, traits::patchArgs(std::move(args), context));
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
        StreamResult readState(const SerializeTable *table, void *unit, FormattedSerializeStream &in, CallerHierarchyBasePtr hierarchy)
        {
            CallerHierarchy newHierarchy = hierarchy.append(unit_cast<T *>(unit));
            CallerHierarchyPtr newHierarchyPtr = newHierarchy;

            auto guard = GuardSelector<Configs...>::guard(newHierarchyPtr);
            (void)guard;
            return Serialize::readState(table, unit, in, newHierarchyPtr);
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
    SERIALIZETABLE_END_EX(, T)

#define FIELD_EX(Idx, ...) \
    SERIALIZETABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Serialize::__serialize_impl__::field<Ty, &Ty::__VA_ARGS__>(STRINGIFY2(FIRST(__VA_ARGS__)), parentConfigs)))

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
