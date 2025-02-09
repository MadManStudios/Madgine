#pragma once


#include "configs/configselector.h"
#include "configs/creator.h"
#include "configs/guard.h"
#include "configs/tags.h"

#include "hierarchy/serializableunitptr.h"

#include "streams/formattedmessagestream.h"

#include "Generic/container/atomiccontaineroperation.h"

#include "container/physical.h"

#include "hierarchy/statetransmissionflags.h"

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
        friend StreamResult tag_invoke(apply_map_t cpo, T *&p, FormattedSerializeStream &in, bool success = true, const CallerHierarchyBasePtr &hierarchy = {})
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
                            STREAM_PROPAGATE_ERROR(convertSyncablePtr(in, id, unit, type));
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
                            STREAM_PROPAGATE_ERROR(convertSerializablePtr(in, id, unit));
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
        friend StreamResult tag_invoke(apply_map_t cpo, std::tuple<Ty...> &t, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            return TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return cpo(e, in, success, hierarchy);
                },
                StreamResult {});
        }

        template <typename T1, typename T2>
        friend StreamResult tag_invoke(apply_map_t cpo, std::pair<T1, T2> &p, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(cpo(p.first, in, success, hierarchy));
            return cpo(p.second, in, success, hierarchy);
        }

        template <typename T>
        friend StreamResult tag_invoke(apply_map_t cpo, std::optional<T> &o, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            if (o)
                return cpo(*o, in, success, hierarchy);
            else
                return {};
        }

        template <SerializeRange C>
        friend StreamResult tag_invoke(apply_map_t cpo, C &c, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            for (auto &t : physical(c)) {
                STREAM_PROPAGATE_ERROR(cpo(t, in, success, hierarchy));
            }
            return {};
        }

        template <PrimitiveType T>
            requires(!std::is_const_v<T>)
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            return {};
        }

        template <typename T>
            requires std::is_const_v<T>
        friend StreamResult tag_invoke(apply_map_t cpo, T &t, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            return {};
        }

        template <typename T>
        friend StreamResult tag_invoke(apply_map_t cpo, const std::unique_ptr<T> &p, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {})
        {
            return cpo(*p, in, success, hierarchy);
        }

        template <typename T>
            requires(!tag_invocable<apply_map_t, T &, FormattedSerializeStream &, bool, const CallerHierarchyBasePtr &>)
        StreamResult operator()(T &t, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {}) const
        {
            return SerializableDataPtr { &t }.applyMap(in, success, hierarchy);
        }

        template <typename T>
        auto operator()(T &item, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy = {}) const
            noexcept(is_nothrow_tag_invocable_v<apply_map_t, T &, FormattedSerializeStream &, bool, const CallerHierarchyBasePtr &>)
                -> tag_invoke_result_t<apply_map_t, T &, FormattedSerializeStream &, bool, const CallerHierarchyBasePtr &>
        {
            return tag_invoke(*this, item, in, success, hierarchy);
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

    template <typename T, typename... Configs>
    void setActive(T &t, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy)
    {
        if constexpr (requires { &Operations<T, Configs...>::setActive; })
            TupleUnpacker::invoke(Operations<T, Configs...>::setActive, t, active, existenceChanged, hierarchy);
    }

    template <typename... Configs, typename T>
    StreamResult readState(FormattedSerializeStream &in, T &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {}, StateTransmissionFlags flags = 0)
    {
        return TupleUnpacker::invoke(Operations<T, Configs...>::read, in, t, name, hierarchy, flags);
    }

    template <typename T, typename... Configs>
    StreamResult read(FormattedSerializeStream &in, T &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {}, StateTransmissionFlags flags = 0)
    {
        SerializableListHolder holder { in };

        if (flags & StateTransmissionFlags_Activation)
            setActive(t, false, false);

        StreamResult result = readState<Configs...>(in, t, name, hierarchy, flags);

        if (flags & StateTransmissionFlags_ApplyMap) {
            assert(in.manager());
            STREAM_PROPAGATE_ERROR(apply_map(t, in, result.mState == StreamState::OK));
        }

        if (flags & StateTransmissionFlags_Activation)
            setActive(t, true, false);

        return result;
    }

    template <typename T, typename... Configs>
    void writeState(FormattedSerializeStream &out, const T &t, const char *name, const CallerHierarchyBasePtr &hierarchy, StateTransmissionFlags flags)
    {
        TupleUnpacker::invoke(Operations<T, Configs...>::write, out, t, name, hierarchy, flags);
    }

    template <typename T, typename... Configs>
    void write(FormattedSerializeStream &out, const T &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {}, StateTransmissionFlags flags = 0)
    {
        SerializableMapHolder holder { out };

        writeState<T, Configs...>(out, t, name, hierarchy);
    }

    template <typename T, typename... Configs>
    StreamResult readAction(T &t, FormattedMessageStream &in, PendingRequest &request, const CallerHierarchyBasePtr &hierarchy = {})
    {
        auto guard = GuardSelector<Configs...>::guard(hierarchy);
        return Operations<T, Configs...>::readAction(t, in, request, hierarchy);
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
    void writeRequest(const T &t, FormattedMessageStream &out, Payload &&payload, const CallerHierarchyBasePtr &hierarchy = {})
    {
        Operations<T, Configs...>::writeRequest(t, out, std::forward<Payload>(payload), hierarchy);
    }

    template <typename T, typename... Configs>
    StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        return Operations<T, Configs...>::visitStream(in, name, visitor);
    }

    template <typename Compound, typename Primitive, typename F>
        requires(!Reference<F> && PrimitiveType<Primitive>)
    StreamResult scanPrimitive(FormattedSerializeStream &in, const char *name, F &&callback)
    {
        return visitStream<Compound>(in, name, StreamVisitorImpl { [callback { std::move(callback) }](PrimitiveHolder<Primitive>, FormattedSerializeStream &stream, const char *name, std::span<std::string_view> tags) -> StreamResult {
            Primitive v;
            STREAM_PROPAGATE_ERROR(stream.readPrimitive(v, name));
            callback(v, tags);
            return {};
        } });
    }

    template <typename Compound, typename TargetCompound, typename F>
        requires(!Reference<F> && !PrimitiveType<TargetCompound>)
    StreamResult scanCompound(FormattedSerializeStream &in, const char *name, F &&callback)
    {
        using BaseType = std::conditional_t<std::derived_from<TargetCompound, SyncableUnitBase>, SyncableUnitBase, SerializableDataPtr>;
        const StreamVisitor *genericVisitor;
        StreamVisitorImpl visitor {
            [&, callback { std::move(callback) }](PrimitiveHolder<BaseType> holder, FormattedSerializeStream &stream, const char *name, std::span<std::string_view> tags) -> StreamResult {
                if (holder.mTable == &serializeTable<TargetCompound>()) {
                    return callback(stream, name);
                } else {
                    //if constexpr (std::same_as<BaseType, SyncableUnitBase>) {
                        // return SyncableUnitBase::visitStream(holder.mTable, stream, name, *genericVisitor);
                        throw "TODO";
                    //} else {
                        //return SerializableDataPtr::visitStream(holder.mTable, stream, name, *genericVisitor);
                   // }
                }
            }
        };
        genericVisitor = &visitor;
        return visitStream<Compound>(in, name, visitor);
    }

    template <typename T, typename... Configs>
    struct Operations {
        static StreamResult read(FormattedSerializeStream &in, T &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {}, StateTransmissionFlags flags = 0)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (PrimitiveType<T>) {
                return in.readPrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                return t.readState(in, name, hierarchy, flags);
            } else  {
                return SerializableDataPtr { &t }.readState(in, name, hierarchy, flags);
            } 
        }

        static void write(FormattedSerializeStream &out, const T &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {}, StateTransmissionFlags flags = 0)
        {
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
            } else if constexpr (PrimitiveType<T>) {
                out.writePrimitive(t, name);
                // mLog.log(t);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                t.writeState(out, name, hierarchy, flags);
            } else {
                SerializableDataConstPtr { &t }.writeState(out, name, CallerHierarchyPtr { hierarchy }, flags);
            } 
        }

        static void setActive(T &item, bool active, bool existenceChanged)
        {
            if constexpr (std::derived_from<T, SyncableUnitBase>) {
                item.setActive(active, existenceChanged);
            } else if constexpr (std::derived_from<T, SerializableUnitBase>) {
                SerializableUnitPtr { &item }.setActive(active, existenceChanged);
            } else if constexpr (!PrimitiveType<T> && !std::is_const_v<T>){
                SerializableDataPtr { &item }.setActive(active, existenceChanged);
            }
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            auto tags = TagsSelector<Configs...>::getTags();
            if constexpr (std::is_const_v<T>) {
                // Don't do anything here
                return {};
            } else if constexpr (InstanceOf<T, EnumType>) {
                return visitor.visit(PrimitiveHolder<EnumTag> { &T::Representation::sTable }, in, name, tags);
            } else if constexpr (InstanceOf<T, Flags>) {
                return visitor.visit(PrimitiveHolder<FlagsTag> { &T::Representation::sTable }, in, name, tags);
            } else if constexpr (PrimitiveType<T>) {
                return visitor.visit(PrimitiveHolder<typename PrimitiveReducer<T>::type> {}, in, name, tags);
            } else if constexpr (std::derived_from<T, SyncableUnitBase>) {
                return visitor.visit(PrimitiveHolder<SyncableUnitBase> { &serializeTable<T>() }, in, name, tags);
            } else {
                return visitor.visit(PrimitiveHolder<SerializableDataPtr> { &serializeTable<T>() }, in, name, tags);
            } 
        }
    };

    template <typename T, typename... Configs>
    struct Operations<std::unique_ptr<T>, Configs...> {

        static StreamResult read(FormattedSerializeStream &in, const std::unique_ptr<T> &p, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            return Operations<T, Configs...>::read(in, *p, name, hierarchy);
        }

        static void write(FormattedSerializeStream &out, const std::unique_ptr<T> &p, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            Operations<T, Configs...>::write(out, *p, name, hierarchy);
        }

        static void setSynced(const std::unique_ptr<T> &p, bool b)
        {
            Operations<T, Configs...>::setSynced(*p, b);
        }

        static void setActive(const std::unique_ptr<T> &p, bool active, bool existenceChanged)
        {
            Operations<T, Configs...>::setActive(*p, active, existenceChanged);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            return Operations<T, Configs...>::visitStream(in, name, visitor);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<const std::unique_ptr<T>, Configs...> : Operations<std::unique_ptr<T>, Configs...> {
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty...>, Configs...> {

        static StreamResult read(FormattedSerializeStream &in, std::tuple<Ty...> &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::readState(in, e, nullptr, hierarchy);
                },
                StreamResult {}));
            return in.endContainerRead(name);
        }

        static void write(FormattedSerializeStream &out, const std::tuple<Ty...> &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            out.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::writeState(out, e, "Element", hierarchy);
            });
            out.endContainerWrite(name);
        }

        struct VisitHelper {
            template <typename T>
            StreamResult operator()(StreamResult r)
            {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::visitStream<T>(in, nullptr, visitor);
            }

            FormattedSerializeStream &in;
            const StreamVisitor &visitor;
        };

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TypeUnpacker::accumulate<type_pack<Ty...>>(
                VisitHelper { in, visitor },
                StreamResult {}));
            return in.endContainerRead(name);
        }
    };

    template <typename... Ty, typename... Configs>
    struct Operations<std::tuple<Ty &...>, Configs...> {

        static StreamResult read(FormattedSerializeStream &in, std::tuple<Ty &...> t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TupleUnpacker::accumulate(
                t, [&](auto &e, StreamResult r) {
                    STREAM_PROPAGATE_ERROR(std::move(r));
                    return Serialize::read(in, e, nullptr, hierarchy);
                },
                StreamResult {}));
            return in.endContainerRead(name);
        }

        static void write(FormattedSerializeStream &out, const std::tuple<const Ty &...> t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            out.beginContainerWrite(name);
            TupleUnpacker::forEach(t, [&](const auto &e) {
                Serialize::write(out, e, "Element", hierarchy);
            });
            out.endContainerWrite(name);
        }

        struct VisitHelper {
            template <typename T>
            StreamResult operator()(StreamResult r)
            {
                STREAM_PROPAGATE_ERROR(std::move(r));
                return Serialize::visitStream<T>(in, nullptr, visitor);
            }

            FormattedSerializeStream &in;
            const StreamVisitor &visitor;
        };

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, false));
            STREAM_PROPAGATE_ERROR(TypeUnpacker::accumulate<type_pack<Ty...>>(
                VisitHelper { in, visitor },
                StreamResult {}));
            return in.endContainerRead(name);
        }
    };

    template <typename U, typename V, typename... Configs>
    struct Operations<std::pair<U, V>, Configs...> {

        static StreamResult read(FormattedSerializeStream &in, std::pair<U, V> &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::readState<U>(in, t.first, nullptr, hierarchy));
            STREAM_PROPAGATE_ERROR(Serialize::readState<V>(in, t.second, nullptr, hierarchy));
            return in.endCompoundRead(name);
        }

        static void write(FormattedSerializeStream &out, const std::pair<U, V> &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            out.beginCompoundWrite(name);
            Serialize::writeState<U>(out, t.first, "First", hierarchy);
            Serialize::writeState<V>(out, t.second, "Second", hierarchy);
            out.endCompoundWrite(name);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<U>(in, nullptr, visitor));
            STREAM_PROPAGATE_ERROR(Serialize::visitStream<V>(in, nullptr, visitor));
            return in.endCompoundRead(name);
        }
    };

    template <typename T, typename... Configs>
    struct Operations<std::optional<T>, Configs...> {

        static StreamResult read(FormattedSerializeStream &in, std::optional<T> &p, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            bool hasValue;
            STREAM_PROPAGATE_ERROR(readState(in, hasValue, "value"));
            if (!hasValue) {
                p.reset();
                in.beginCompoundRead(name);
                in.endCompoundRead(name);
                return {};
            } else {
                p.emplace();
                return readState(in, *p, name, hierarchy);
            }            
        }

        static void write(FormattedSerializeStream &out, const std::optional<T> &p, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            out.beginExtendedWrite(name, 1);
            writeState(out, p.has_value(), "value", hierarchy);
            if (p) {
                writeState(out, *p, name, hierarchy);
            } else {
                out.beginCompoundWrite(name);
                out.endCompoundWrite(name);
            }
        }
    };

}
}