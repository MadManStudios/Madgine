#pragma once

#include "Generic/containers/atomiccontaineroperation.h"

#include "configs/configselector.h"
#include "configs/guard.h"
#include "configs/tags.h"
#include "container/physical.h"
#include "context.h"
#include "hierarchy/serializableunitptr.h"
#include "streams/formattedmessagestream.h"
#include "streams/serializablemapholder.h"
#include "visitor.h"

namespace Engine {
namespace Serialize {

    META_EXPORT StreamResult convertSyncablePtr(FormattedSerializeStream &in, UnitId id, SyncableUnitBase *&out, const SerializeTable *&type);
    META_EXPORT StreamResult convertSerializablePtr(FormattedSerializeStream &in, uint32_t id, SerializableDataPtr &out);

    template <typename C>
    concept SerializeRange = std::ranges::range<C> && !PrimitiveType<C>;

    struct set_parent_t {

        template <SerializeRange C>
        friend void tag_invoke(set_parent_t cpo, C &c, SerializableUnitBase *parent)
        {
            for (auto &t : physical(c)) {
                cpo(t, parent);
            }
        }

        template <typename T>
            requires tag_invocable<set_parent_t, T &, SerializableUnitBase *>
        friend void tag_invoke(set_parent_t cpo, const std::unique_ptr<T> &p, SerializableUnitBase *parent)
        {
            tag_invoke(cpo, *p, parent);
        }

        template <typename T>
            requires(!is_tag_invocable_v<set_parent_t, T &, void *>)
        void operator()(T &t, void *parent) const
        {
        }

        template <typename T>
            requires(!is_tag_invocable_v<set_parent_t, T &, SerializableUnitBase *>)
        void operator()(T &t, SerializableUnitBase *parent) const
        {
        }

        template <typename T>
        auto operator()(T &item, SerializableUnitBase *parent) const
            noexcept(is_nothrow_tag_invocable_v<set_parent_t, T &, SerializableUnitBase *>)
                -> tag_invoke_result_t<set_parent_t, T &, SerializableUnitBase *>
        {
            return tag_invoke(*this, item, parent);
        }
    };

    inline constexpr set_parent_t set_parent;

    struct apply_map_t {

        template <typename... Ty, typename Context>
        friend StreamResult tag_invoke(apply_map_t cpo, std::tuple<Ty...> &t, FormattedSerializeStream &in, bool success, Context &&context)
        {
            return TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return cpo(e, in, success);
                },
                StreamResult {});
        }

        template <typename T1, typename T2, typename Context>
        friend StreamResult tag_invoke(apply_map_t cpo, std::pair<T1, T2> &p, FormattedSerializeStream &in, bool success, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(cpo(p.first, in, success));
            return cpo(p.second, in, success);
        }

        template <typename T, typename Context>
        friend StreamResult tag_invoke(apply_map_t cpo, std::optional<T> &o, FormattedSerializeStream &in, bool success, Context &&context)
        {
            if (o)
                return cpo(*o, in, success);
            else
                return {};
        }

        template <SerializeRange C, typename Context>
        friend StreamResult tag_invoke(apply_map_t cpo, C &c, FormattedSerializeStream &in, bool success, Context &&context)
        {
            for (auto &t : physical(c)) {
                STREAM_PROPAGATE_ERROR(cpo(t, in, success, context));
            }
            return {};
        }

        template <PrimitiveType T, typename Context>
            requires(!std::is_const_v<T> && !std::is_pointer_v<T>)
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, FormattedSerializeStream &in, bool success, Context &&context)
        {
            return {};
        }

        template <typename T, typename Context>
            requires std::is_const_v<T>
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, FormattedSerializeStream &in, bool success, Context &&context)
        {
            return {};
        }

        template <typename T, typename Context>
        friend StreamResult tag_invoke(apply_map_t cpo, const std::unique_ptr<T> &p, FormattedSerializeStream &in, bool success, Context &&context)
        {
            return cpo(*p, in, success);
        }

        template <typename T, typename Context = ContextPtr>
            requires(!tag_invocable<apply_map_t, T &, FormattedSerializeStream &, bool, Context>)
        StreamResult operator()(T &t, FormattedSerializeStream &in, bool success, Context &&context = {}) const
        {
            if constexpr (std::is_pointer_v<T>) {
                if (success) {
                    uint32_t ptr = reinterpret_cast<uintptr_t>(t);
                    if (ptr & 0x3) {
                        switch (static_cast<UnitIdTag>(ptr & 0x3)) {
                        case UnitIdTag::SYNCABLE:
                            if constexpr (std::derived_from<std::remove_pointer_t<T>, SyncableUnitBase>) {
                                UnitId id = (ptr >> 2);
                                SyncableUnitBase *unit;
                                const SerializeTable *type;
                                STREAM_PROPAGATE_ERROR(convertSyncablePtr(in, id, unit, type));
                                if (type != &serializeTable<std::remove_pointer_t<T>>())
                                    throw 0;
                                t = static_cast<T>(unit);
                            } else {
                                throw 0;
                            }
                            break;
                        case UnitIdTag::SERIALIZABLE:
                            if constexpr (!std::derived_from<std::remove_pointer_t<T>, SyncableUnitBase>) {
                                uint32_t id = (ptr >> 2);
                                SerializableDataPtr unit;
                                STREAM_PROPAGATE_ERROR(convertSerializablePtr(in, id, unit));
                                static_assert(!std::same_as<T, SerializableUnitBase*>);
                                if (unit.mType != &serializeTable<std::remove_pointer_t<T>>())
                                    throw 0;
                                t = static_cast<T>(unit.unit());
                            } else {
                                throw 0;
                            }
                            break;
                        default:
                            throw 0;
                        }
                    }
                } else {
                    t = nullptr;
                }
                return {};
            } else {
                return SerializableDataPtr { &t }.applyMap(in, success, context);
            }
        }

        template <typename T, typename Context = ContextPtr>
        auto operator()(T &item, FormattedSerializeStream &in, bool success, Context &&context = {}) const
            noexcept(is_nothrow_tag_invocable_v<apply_map_t, T &, FormattedSerializeStream &, bool, Context>)
                -> tag_invoke_result_t<apply_map_t, T &, FormattedSerializeStream &, bool, Context>
        {
            return tag_invoke(*this, item, in, success, context);
        }
    };

    inline constexpr apply_map_t apply_map;

    struct set_synced_t {

        template <SerializeRange C, typename Context>
        friend void tag_invoke(set_synced_t cpo, C &c, bool b, Context &&context)
        {
            for (auto &t : physical(c)) {
                cpo(t, b, std::forward<Context>(context));
            }
        }

        template <typename T, typename Context>
            requires tag_invocable<set_synced_t, T &, bool, Context>
        friend void tag_invoke(set_synced_t cpo, const std::unique_ptr<T> &p, bool b, Context &&context = {})
        {
            tag_invoke(cpo, *p, b, std::forward<Context>(context));
        }

        template <typename T, typename Context = ContextPtr>
            requires(!is_tag_invocable_v<set_synced_t, T &, bool, Context>)
        void operator()(T &t, bool b, Context &&context = {}) const
        {
        }

        template <typename T, typename Context = ContextPtr>
        auto operator()(T &item, bool b, Context &&context = {}) const
            noexcept(is_nothrow_tag_invocable_v<set_synced_t, T &, bool, Context>)
                -> tag_invoke_result_t<set_synced_t, T &, bool, Context>
        {
            return tag_invoke(*this, item, b, std::forward<Context>(context));
        }
    };

    inline constexpr set_synced_t set_synced;

    template <typename... Configs>
    struct set_active_t {

        template <typename U, typename V, typename Context>
        friend void tag_invoke(set_active_t cpo, std::pair<U, V> &p, bool active, bool existenceChanged, Context &&context)
        {
            cpo(p.first, active, existenceChanged, context);
            cpo(p.second, active, existenceChanged, context);
        }

        template <typename T, typename Context>
        friend void tag_invoke(set_active_t cpo, const std::unique_ptr<T> &p, bool active, bool existenceChanged, Context &&context)
        {
            cpo(*p, active, existenceChanged, context);
        }

        template <SerializeRange C, typename Context>
            requires(!requires { typename C::is_serializable_container; })
        friend void tag_invoke(set_active_t cpo, C &c, bool active, bool existenceChanged, Context &&context)
        {
            for (auto &t : physical(c)) {
                cpo(t, active, existenceChanged, context);
            }
        }

        template <PrimitiveType T, typename Context>
        friend void tag_invoke(set_active_t cpo, T &t, bool active, bool existenceChanged, Context &&context)
        {
        }

        template <typename T, typename Context = ContextPtr>
            requires(!is_tag_invocable_v<set_active_t, T &, bool, bool, Context>)
        void operator()(T &item, bool active, bool existenceChanged, Context &&context = {}) const
        {
            if constexpr (!std::is_const_v<T>)
                SerializableDataPtr { &item }.setActive(active, existenceChanged, context);
        }

        template <typename T, typename Context = ContextPtr>
        auto operator()(T &item, bool active, bool existenceChanged, Context &&context = {}) const
            noexcept(is_nothrow_tag_invocable_v<set_active_t, T &, bool, bool, Context>)
                -> tag_invoke_result_t<set_active_t, T &, bool, bool, Context>
        {
            return tag_invoke(*this, item, active, existenceChanged, context);
        }
    };

    template <typename... Configs>
    inline constexpr set_active_t<Configs...> set_active;

    template <typename T, typename... Configs, typename Context = ContextPtr>
    StreamResult readState(FormattedSerializeStream &in, T &t, const char *name, Context &&context = {})
    {
        set_active<Configs...>(t, false, false, context);

        StreamResult result = read(in, t, name, context);

        assert(in.manager());
        STREAM_PROPAGATE_ERROR(apply_map(t, in, result.mState == StreamState::OK));

        set_active<Configs...>(t, true, false, context);

        return result;
    }

    template <typename T, typename... Configs, typename Context>
    StreamResult read(FormattedSerializeStream &in, T &t, const char *name, Context &&context)
    {
        return Operations<T, Configs...>::read(in, t, name, context);
    }

    template <typename T, typename... Configs, typename Context>
    void write(FormattedSerializeStream &out, const T &t, const char *name, Context &&context)
    {
        Operations<T, Configs...>::write(out, t, name, context);
    }

    template <typename T, typename... Configs, typename Context>
    StreamResult readAction(T &t, FormattedSerializeStream &in, PendingRequest &request, Context &&context)
    {
        [[maybe_unused]] auto guard = GuardSelector<Configs...>::guard(context);
        return Operations<T, Configs...>::readAction(t, in, request, context);
    }

    template <typename T, typename... Configs, typename Context>
    StreamResult readRequest(T &t, FormattedMessageStream &inout, MessageId id, Context &&context = {})
    {
        return Operations<T, Configs...>::readRequest(t, inout, id, context);
    }

    template <typename T, typename... Configs, typename Payload, typename Context>
    void writeAction(const T &t, const std::vector<WriteMessage> &outStreams, Payload &&payload, Context &&context = {})
    {
        Operations<T, Configs...>::writeAction(t, outStreams, std::forward<Payload>(payload), context);
    }

    template <typename T, typename... Configs, typename Payload, typename Context>
    void writeRequest(const T &t, FormattedSerializeStream &out, Payload &&payload, Context &&context)
    {
        Operations<T, Configs...>::writeRequest(t, out, std::forward<Payload>(payload), context);
    }

    template <typename T, typename... Configs>
    StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return Operations<T, Configs...>::visitStream(in, name, visitor, depth);
    }

    template <typename... Configs, typename T>
    StreamResult visitStream(PrimitiveHolder<T> holder, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        auto tags = TagsSelector<Configs...>::getTags();
        return visitor.visit(holder, in, name, tags, depth);
    }

    template <typename Compound, typename Primitive, typename F>
        requires(!Concepts::Reference<F> && PrimitiveType<Primitive>)
    StreamResult scanPrimitive(FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        return visitStream<Compound>(in, name, StreamVisitorImpl { [callback { std::move(callback) }](PrimitiveHolder<Primitive>, FormattedSerializeStream &stream, const char *name, std::span<std::string_view> tags, size_t depth) -> StreamResult {
            Primitive v;
            STREAM_PROPAGATE_ERROR(stream.readPrimitive(v, name));
            callback(v, name, tags, depth);
            return {};
        } },
            depth);
    }

    template <typename Primitive, typename F, typename T>
        requires(!Concepts::Reference<F> && PrimitiveType<Primitive>)
    StreamResult scanPrimitive(PrimitiveHolder<T> holder, FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        return visitStream(holder, in, name, StreamVisitorImpl { [callback { std::move(callback) }](PrimitiveHolder<Primitive>, FormattedSerializeStream &stream, const char *name, std::span<std::string_view> tags, size_t depth) -> StreamResult {
            Primitive v;
            STREAM_PROPAGATE_ERROR(stream.readPrimitive(v, name));
            callback(v, name, tags, depth);
            return {};
        } },
            depth);
    }

    template <typename Compound, typename TargetCompound, typename... Configs, typename F>
        requires(!Concepts::Reference<F> && !PrimitiveType<TargetCompound>)
    StreamResult scanCompound(FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        using BaseType = std::conditional_t<std::derived_from<TargetCompound, SyncableUnitBase>, SyncableUnitBase, DataTag>;
        StreamVisitorImpl visitor {
            [&, callback { std::move(callback) }](PrimitiveHolder<BaseType> holder, FormattedSerializeStream &stream, const char *name, std::span<std::string_view> tags, size_t depth) -> std::optional<StreamResult> {
                if (holder.mTable == &serializeTable<TargetCompound>()) {
                    return callback(stream, name, depth);
                } else {
                    return {};
                }
            }
        };
        return visitStream<Compound, Configs...>(in, name, visitor, depth);
    }

    template <typename T, typename... Configs>
    struct Operations {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, T &t, const char *name, Context &&context)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (PrimitiveType<T>) {
                return in.readPrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                return t.readState(in, name, context);
            } else {
                return SerializableDataPtr { &t }.readState(in, name, false, context);
            }
        }

        template <typename Context>
        static void write(FormattedSerializeStream &out, const T &t, const char *name, Context &&context)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
            } else if constexpr (PrimitiveType<T>) {
                out.writePrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                t.writeState(out, name, context);
            } else {
                SerializableDataConstPtr { &t }.writeState(out, name, false, context);
            }
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            auto tags = TagsSelector<Configs...>::getTags();
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (Concepts::InstanceOf<T, EnumImpl>) {
                return visitor.visit(PrimitiveHolder<EnumTag> { &T::Representation::sTable }, in, name, tags, depth);
            } else if constexpr (Concepts::InstanceOf<T, Flags>) {
                return visitor.visit(PrimitiveHolder<FlagsTag> { &T::Representation::sTable }, in, name, tags, depth);
            } else if constexpr (PrimitiveType<T>) {
                return visitor.visit(PrimitiveHolder<typename PrimitiveReducer<T>::type> {}, in, name, tags, depth);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                return visitor.visit(PrimitiveHolder<SyncableUnitBase> { &serializeTable<T>() }, in, name, tags, depth);
            } else {
                return visitor.visit(PrimitiveHolder<DataTag> { &serializeTable<T>() }, in, name, tags, depth);
            }
        }
    };

    template <typename T, typename... Configs>
    struct Operations<std::unique_ptr<T>, Configs...> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, const std::unique_ptr<T> &p, const char *name, Context &&context)
        {
            return Operations<T, Configs...>::read(in, *p, name, context);
        }
        template <typename Context>
        static void write(FormattedSerializeStream &out, const std::unique_ptr<T> &p, const char *name, Context &&context)
        {
            Operations<T, Configs...>::write(out, *p, name, context);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            return Operations<T, Configs...>::visitStream(in, name, visitor, depth);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<const std::unique_ptr<T>, Configs...> : Operations<std::unique_ptr<T>, Configs...> {
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty...>, Configs...> {

        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, std::tuple<Ty...> &t, const char *name, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::read(in, e, nullptr, context);
                },
                StreamResult {}));
            return in.endContainerRead(name);
        }

        template <typename Context>
        static void write(FormattedSerializeStream &out, const std::tuple<Ty...> &t, const char *name, Context &&context)
        {
            out.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::write(out, e, "Element", context);
            });
            out.endContainerWrite(name);
        }

        struct VisitHelper {
            template <typename T>
            StreamResult operator()(StreamResult r)
            {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::visitStream<T>(in, nullptr, visitor, depth);
            }

            FormattedSerializeStream &in;
            const StreamVisitor &visitor;
            size_t depth;
        };

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TypeUnpacker::accumulate<type_pack<Ty...>>(
                VisitHelper { in, visitor, depth + 1 },
                StreamResult {}));
            return in.endContainerRead(name);
        }
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty &...>, Configs...> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, std::tuple<Ty &...> t, const char *name, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::read(in, e, nullptr, context);
                },
                StreamResult {}));
            return in.endContainerRead(name);
        }
        template <typename Context>
        static void write(FormattedSerializeStream &out, const std::tuple<const Ty &...> t, const char *name, Context &&context)
        {
            out.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::write(out, e, "Element", context);
            });
            out.endContainerWrite(name);
        }

        struct VisitHelper {
            template <typename T>
            StreamResult operator()(StreamResult r)
            {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::visitStream<T>(in, nullptr, visitor, depth);
            }

            FormattedSerializeStream &in;
            const StreamVisitor &visitor;
            size_t depth;
        };

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TypeUnpacker::accumulate<type_pack<Ty...>>(
                VisitHelper { in, visitor, depth + 1 },
                StreamResult {}));
            return in.endContainerRead(name);
        }
    };

    template <typename U, typename V, typename... Configs>
    struct Operations<std::pair<U, V>, Configs...> {

        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, std::pair<U, V> &t, const char *name, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::read<U>(in, t.first, nullptr, context));
            STREAM_PROPAGATE_ERROR(Serialize::read<V>(in, t.second, nullptr, context));
            return in.endCompoundRead(name);
        }

        template <typename Context>
        static void write(FormattedSerializeStream &out, const std::pair<U, V> &t, const char *name, Context &&context)
        {
            out.beginCompoundWrite(name);
            Serialize::write<U>(out, t.first, "First", context);
            Serialize::write<V>(out, t.second, "Second", context);
            out.endCompoundWrite(name);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<U>(in, nullptr, visitor, depth + 1));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<V>(in, nullptr, visitor, depth + 1));
            return in.endCompoundRead(name);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<std::optional<T>, Configs...> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, std::optional<T> &p, const char *name, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            bool hasValue;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, hasValue, "value"));
            if (!hasValue) {
                p.reset();
                STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
                STREAM_PROPAGATE_ERROR(in.endCompoundRead(name));
                return {};
            } else {
                p.emplace();
                return Serialize::read(in, *p, name, context);
            }
        }
        template <typename Context>
        static void write(FormattedSerializeStream &out, const std::optional<T> &p, const char *name, Context &&context)
        {
            out.beginExtendedWrite(name, 1);
            Serialize::write(out, p.has_value(), "value");
            if (p) {
                Serialize::write(out, *p, name, context);
            } else {
                out.beginCompoundWrite(name);
                out.endCompoundWrite(name);
            }
        }
    };

}
}