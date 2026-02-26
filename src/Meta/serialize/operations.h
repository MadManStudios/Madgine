#pragma once

#include "Generic/container/atomiccontaineroperation.h"

#include "configs/configselector.h"
#include "configs/creator.h"
#include "configs/guard.h"
#include "configs/tags.h"
#include "container/physical.h"
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
        template <typename T>
        friend StreamResult tag_invoke(apply_map_t cpo, T *&p, CallerHierarchyFormattedSerializeStream in, bool success = true)
        {
            if (success) {
                uint32_t ptr = reinterpret_cast<uintptr_t>(p);
                if (ptr & 0x3) {
                    switch (static_cast<UnitIdTag>(ptr & 0x3)) {
                    case UnitIdTag::SYNCABLE:
                        if constexpr (std::derived_from<T, SyncableUnitBase>) {
                            UnitId id = (ptr >> 2);
                            SyncableUnitBase *unit;
                            const SerializeTable *type;
                            STREAM_PROPAGATE_ERROR(convertSyncablePtr(in.mStream, id, unit, type));
                            if (type != &serializeTable<T>())
                                throw 0;
                            p = static_cast<T *>(unit);
                        } else {
                            throw 0;
                        }
                        break;
                    case UnitIdTag::SERIALIZABLE:
                        if constexpr (!std::derived_from<T, SyncableUnitBase>) {
                            uint32_t id = (ptr >> 2);
                            SerializableDataPtr unit;
                            STREAM_PROPAGATE_ERROR(convertSerializablePtr(in.mStream, id, unit));
                            static_assert(!std::same_as<T, SerializableUnitBase>);
                            if (unit.mType != &serializeTable<T>())
                                throw 0;
                            p = static_cast<T *>(unit.unit());
                        } else {
                            throw 0;
                        }
                        break;
                    default:
                        throw 0;
                    }
                }
            } else {
                p = nullptr;
            }
            return {};
        }

        template <typename... Ty>
        friend StreamResult tag_invoke(apply_map_t cpo, std::tuple<Ty...> &t, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            return TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return cpo(e, in, success);
                },
                StreamResult {});
        }

        template <typename T1, typename T2>
        friend StreamResult tag_invoke(apply_map_t cpo, std::pair<T1, T2> &p, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            STREAM_PROPAGATE_ERROR(cpo(p.first, in, success));
            return cpo(p.second, in, success);
        }

        template <typename T>
        friend StreamResult tag_invoke(apply_map_t cpo, std::optional<T> &o, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            if (o)
                return cpo(*o, in, success);
            else
                return {};
        }

        template <SerializeRange C>
        friend StreamResult tag_invoke(apply_map_t cpo, C &c, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            for (auto &t : physical(c)) {
                STREAM_PROPAGATE_ERROR(cpo(t, in, success));
            }
            return {};
        }

        template <PrimitiveType T>
            requires(!std::is_const_v<T>)
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            return {};
        }

        template <typename T>
            requires std::is_const_v<T>
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            return {};
        }

        template <typename T>
        friend StreamResult tag_invoke(apply_map_t cpo, const std::unique_ptr<T> &p, CallerHierarchyFormattedSerializeStream in, bool success)
        {
            return cpo(*p, in, success);
        }

        template <typename T>
            requires(!tag_invocable<apply_map_t, T &, CallerHierarchyFormattedSerializeStream, bool>)
        StreamResult operator()(T &t, CallerHierarchyFormattedSerializeStream in, bool success) const
        {
            return SerializableDataPtr { &t }.applyMap(in, success);
        }

        template <typename T>
        auto operator()(T &item, CallerHierarchyFormattedSerializeStream in, bool success) const
            noexcept(is_nothrow_tag_invocable_v<apply_map_t, T &, CallerHierarchyFormattedSerializeStream, bool>)
                -> tag_invoke_result_t<apply_map_t, T &, CallerHierarchyFormattedSerializeStream, bool>
        {
            return tag_invoke(*this, item, in, success);
        }
    };

    inline constexpr apply_map_t apply_map;

    struct set_synced_t {

        template <SerializeRange C>
        friend void tag_invoke(set_synced_t cpo, C &c, bool b, const CallerHierarchyBasePtr &hierarchy)
        {
            for (auto &t : physical(c)) {
                cpo(t, b, hierarchy);
            }
        }

        template <typename T>
            requires tag_invocable<set_synced_t, T &, bool, const CallerHierarchyBasePtr &>
        friend void tag_invoke(set_synced_t cpo, const std::unique_ptr<T> &p, bool b, const CallerHierarchyBasePtr &hierarchy = {})
        {
            tag_invoke(cpo, *p, b, hierarchy);
        }

        template <typename T>
            requires(!is_tag_invocable_v<set_synced_t, T &, bool, const CallerHierarchyBasePtr &>)
        void operator()(T &t, bool b, const CallerHierarchyBasePtr &hierarchy = {}) const
        {
        }

        template <typename T>
        auto operator()(T &item, bool b, const CallerHierarchyBasePtr &hierarchy = {}) const
            noexcept(is_nothrow_tag_invocable_v<set_synced_t, T &, bool, const CallerHierarchyBasePtr &>)
                -> tag_invoke_result_t<set_synced_t, T &, bool, const CallerHierarchyBasePtr &>
        {
            return tag_invoke(*this, item, b, hierarchy);
        }
    };

    inline constexpr set_synced_t set_synced;

    template <typename... Configs>
    struct set_active_t {

        template <typename U, typename V>
        friend void tag_invoke(set_active_t cpo, std::pair<U, V> &p, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy = {})
        {
            cpo(p.first, active, existenceChanged, hierarchy);
            cpo(p.second, active, existenceChanged, hierarchy);
        }

        template <typename T>
        friend void tag_invoke(set_active_t cpo, const std::unique_ptr<T> &p, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy = {})
        {
            cpo(*p, active, existenceChanged, hierarchy);
        }

        template <SerializeRange C>
            requires(!requires { typename C::is_serializable_container; })
        friend void tag_invoke(set_active_t cpo, C &c, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy = {})
        {
            for (auto &t : physical(c)) {
                cpo(t, active, existenceChanged);
            }
        }

        template <PrimitiveType T>
        friend void tag_invoke(set_active_t cpo, T &t, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy)
        {
        }

        template <typename T>
            requires(!is_tag_invocable_v<set_active_t, T &, bool, bool, const CallerHierarchyBasePtr &>)
        void operator()(T &item, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy = {}) const
        {
            if constexpr (!std::is_const_v<T>)
                SerializableDataPtr { &item }.setActive(active, existenceChanged);
        }

        template <typename T>
        auto operator()(T &item, bool active, bool existenceChanged, const CallerHierarchyBasePtr &hierarchy = {}) const
            noexcept(is_nothrow_tag_invocable_v<set_active_t, T &, bool, bool, const CallerHierarchyBasePtr &>)
                -> tag_invoke_result_t<set_active_t, T &, bool, bool, const CallerHierarchyBasePtr &>
        {
            return tag_invoke(*this, item, active, existenceChanged, hierarchy);
        }
    };

    template <typename... Configs>
    inline constexpr set_active_t<Configs...> set_active;

    template <typename T, typename... Configs>
    StreamResult readState(CallerHierarchyFormattedSerializeStream in, T &t, const char *name)
    {
        set_active<Configs...>(t, false, false);

        StreamResult result = read(in, t, name);

        assert(in.mStream.manager());
        STREAM_PROPAGATE_ERROR(apply_map(t, in, result.mState == StreamState::OK));

        set_active<Configs...>(t, true, false);

        return result;
    }

    template <typename T, typename... Configs>
    StreamResult read(CallerHierarchyFormattedSerializeStream in, T &t, const char *name)
    {
        return TupleUnpacker::invoke(Operations<T, Configs...>::read, in, t, name);
    }

    template <typename T, typename... Configs>
    void write(CallerHierarchyFormattedSerializeStream out, const T &t, const char *name)
    {
        TupleUnpacker::invoke(Operations<T, Configs...>::write, out, t, name);
    }

    template <typename T, typename... Configs>
    StreamResult readAction(T &t, CallerHierarchyFormattedSerializeStream in, PendingRequest &request)
    {
        [[maybe_unused]] auto guard = GuardSelector<Configs...>::guard(in.mHierarchy);
        return Operations<T, Configs...>::readAction(t, in, request);
    }

    template <typename T, typename... Configs>
    StreamResult readRequest(T &t, FormattedMessageStream &inout, MessageId id, const CallerHierarchyBasePtr &hierarchy = {})
    {
        return Operations<T, Configs...>::readRequest(t, inout, id, hierarchy);
    }

    template <typename T, typename... Configs, typename Payload>
    void writeAction(const T &t, const std::vector<WriteMessage> &outStreams, Payload &&payload, const CallerHierarchyBasePtr &hierarchy = {})
    {
        Operations<T, Configs...>::writeAction(t, outStreams, std::forward<Payload>(payload), hierarchy);
    }

    template <typename T, typename... Configs, typename Payload>
    void writeRequest(const T &t, CallerHierarchyFormattedSerializeStream out, Payload &&payload)
    {
        Operations<T, Configs...>::writeRequest(t, out, std::forward<Payload>(payload));
    }

    template <typename T, typename... Configs>
    StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
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
        requires(!Reference<F> && PrimitiveType<Primitive>)
    StreamResult scanPrimitive(FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        return visitStream<Compound>(in, name, StreamVisitorImpl { [callback { std::move(callback) }](PrimitiveHolder<Primitive>, CallerHierarchyFormattedSerializeStream stream, const char *name, std::span<std::string_view> tags, size_t depth) -> StreamResult {
            Primitive v;
            STREAM_PROPAGATE_ERROR(stream.mStream.readPrimitive(v, name));
            callback(v, name, tags, depth);
            return {};
        } },
            depth);
    }

    template <typename Primitive, typename F, typename T>
        requires(!Reference<F> && PrimitiveType<Primitive>)
    StreamResult scanPrimitive(PrimitiveHolder<T> holder, FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        return visitStream(holder, in, name, StreamVisitorImpl { [callback { std::move(callback) }](PrimitiveHolder<Primitive>, CallerHierarchyFormattedSerializeStream stream, const char *name, std::span<std::string_view> tags, size_t depth) -> StreamResult {
            Primitive v;
            STREAM_PROPAGATE_ERROR(stream.mStream.readPrimitive(v, name));
            callback(v, name, tags, depth);
            return {};
        } },
            depth);
    }

    template <typename Compound, typename TargetCompound, typename... Configs, typename F>
        requires(!Reference<F> && !PrimitiveType<TargetCompound>)
    StreamResult scanCompound(FormattedSerializeStream &in, const char *name, F &&callback, size_t depth = 0)
    {
        using BaseType = std::conditional_t<std::derived_from<TargetCompound, SyncableUnitBase>, SyncableUnitBase, DataTag>;
        StreamVisitorImpl visitor {
            [&, callback { std::move(callback) }](PrimitiveHolder<BaseType> holder, CallerHierarchyFormattedSerializeStream stream, const char *name, std::span<std::string_view> tags, size_t depth) -> std::optional<StreamResult> {
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
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, T &t, const char *name)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (PrimitiveType<T>) {
                return in.mStream.readPrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                return t.readState(in, name);
            } else {
                return SerializableDataPtr { &t }.readState(in, name);
            }
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const T &t, const char *name)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
            } else if constexpr (PrimitiveType<T>) {
                out.mStream.writePrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                t.writeState(out, name);
            } else {
                SerializableDataConstPtr { &t }.writeState(out, name);
            }
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            auto tags = TagsSelector<Configs...>::getTags();
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (InstanceOf<T, EnumImpl>) {
                return visitor.visit(PrimitiveHolder<EnumTag> { &T::Representation::sTable }, in, name, tags, depth);
            } else if constexpr (InstanceOf<T, Flags>) {
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

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, const std::unique_ptr<T> &p, const char *name)
        {
            return Operations<T, Configs...>::read(in, *p, name);
        }

        static void write(CallerHierarchyFormattedSerializeStream &out, const std::unique_ptr<T> &p, const char *name)
        {
            Operations<T, Configs...>::write(out, *p, name);
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            return Operations<T, Configs...>::visitStream(in, name, visitor, depth);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<const std::unique_ptr<T>, Configs...> : Operations<std::unique_ptr<T>, Configs...> {
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty...>, Configs...> {

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, std::tuple<Ty...> &t, const char *name)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::read(in, e, nullptr);
                },
                StreamResult {}));
            return in.mStream.endContainerRead(name);
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const std::tuple<Ty...> &t, const char *name)
        {
            out.mStream.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::write(out, e, "Element");
            });
            out.mStream.endContainerWrite(name);
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

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, std::tuple<Ty &...> t, const char *name)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::read(in, e, nullptr);
                },
                StreamResult {}));
            return in.mStream.endContainerRead(name);
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const std::tuple<const Ty &...> t, const char *name)
        {
            out.mStream.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::write(out, e, "Element");
            });
            out.mStream.endContainerWrite(name);
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

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, std::pair<U, V> &t, const char *name)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::read<U>(in, t.first, nullptr));
            STREAM_PROPAGATE_ERROR(Serialize::read<V>(in, t.second, nullptr));
            return in.mStream.endCompoundRead(name);
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const std::pair<U, V> &t, const char *name)
        {
            out.mStream.beginCompoundWrite(name);
            Serialize::write<U>(out, t.first, "First");
            Serialize::write<V>(out, t.second, "Second");
            out.mStream.endCompoundWrite(name);
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<U>(in, nullptr, visitor, depth + 1));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<V>(in, nullptr, visitor, depth + 1));
            return in.mStream.endCompoundRead(name);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<std::optional<T>, Configs...> {

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, std::optional<T> &p, const char *name)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
            bool hasValue;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, hasValue, "value"));
            if (!hasValue) {
                p.reset();
                STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
                STREAM_PROPAGATE_ERROR(in.mStream.endCompoundRead(name));
                return {};
            } else {
                p.emplace();
                return Serialize::read(in, *p, name);
            }
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const std::optional<T> &p, const char *name)
        {
            out.mStream.beginExtendedWrite(name, 1);
            Serialize::write(out, p.has_value(), "value");
            if (p) {
                Serialize::write(out, *p, name);
            } else {
                out.mStream.beginCompoundWrite(name);
                out.mStream.endCompoundWrite(name);
            }
        }
    };

}
}