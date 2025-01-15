#pragma once

#include "../configs/filter.h"
#include "../primitivetypes.h"
#include "../streams/comparestreamid.h"
#include "Generic/container/containerevent.h"
#include "../configs/requestpolicy.h"

namespace Engine {
namespace Serialize {

    template <typename C, typename... Configs>
    struct ContainerOperations {

        using T = typename C::value_type;

        using Creator = CreatorSelector<Configs...>;
        using Filter = FilterSelector<Configs...>;

        template <typename Op>
        static StreamResult readOp(FormattedSerializeStream &in, Op &op, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, !container_traits<C>::is_fixed_size));

            if constexpr (!container_traits<C>::is_fixed_size) {
                TupleUnpacker::invoke(&Creator::template clear<Op>, op, hierarchy);

                while (in.hasContainerItem()) {
                    std::ranges::iterator_t<Op> it;
                    STREAM_PROPAGATE_ERROR(TupleUnpacker::invoke(&Creator::template readItem<Op>, in, op, it, physical(op).end(), hierarchy));
                }
            } else {
                for (T &t : physical(op)) {
                    if (Filter::filter(t))
                        STREAM_PROPAGATE_ERROR(Serialize::readState(in, t, "Item"));
                }
            }

            return in.endContainerRead(name);
        }

        static StreamResult read(FormattedSerializeStream &in, C &container, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            decltype(auto) op = resetOperation(container, Creator::controlled);
            return readOp(in, op, name, hierarchy);
        }

        static void write(FormattedSerializeStream &out, const C &container, const char *name, const CallerHierarchyBasePtr &hierarchy = {})
        {
            if constexpr (container_traits<C>::is_fixed_size)
                out.beginContainerWrite(name);
            else {
                size_t size;
                if constexpr (std::is_same_v<Filter, DefaultFilter>) {
                    size = physical(container).size();
                } else {
                    size = 0;
                    for (const auto &t : physical(container)) {
                        if (Filter::filter(t))
                            ++size;
                    }
                }
                out.beginContainerWrite(name, size);
            }
            for (const auto &t : physical(container)) {
                if (Filter::filter(t))
                    TupleUnpacker::invoke(&Creator::template writeItem<C>, out, t, hierarchy);
            }
            out.endContainerWrite(name);
        }

        static void setActive(C &c, bool active, bool existenceChanged)
        {
            for (auto &t : physical(c)) {
                Serialize::setActive(t, active, existenceChanged);
            }
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            STREAM_PROPAGATE_ERROR(in.beginContainerRead(name, !container_traits<C>::is_fixed_size));

            while (in.hasContainerItem()) {
                STREAM_PROPAGATE_ERROR(Creator::template visitStream<C>(in, visitor));
            }

            return in.endContainerRead(name);
        }

        static StreamResult readIterator(FormattedSerializeStream &in, C &c, typename C::iterator &it)
        {
            int32_t dist;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, dist, "it"));
            it = std::next(c.begin(), dist);
            return {};
        }

        static void writeIterator(FormattedSerializeStream &out, const C &c, const typename C::const_iterator &it)
        {
            Serialize::write<int32_t>(out, std::distance(c.begin(), it), "it");
        }
    };

    template <SerializeRange C, typename... Configs>
    struct Operations<C, Configs...> : ContainerOperations<C, Configs...> {
    };

    template <typename C>
    concept SerializeWrappedRange = SerializeRange<C> && requires
    {
        typename underlying_container<C>::type;
    };

    template <SerializeWrappedRange C, typename... Configs>
    struct Operations<C, Configs...> : Operations<typename underlying_container<C>::type, Configs...> {
    };

    template <typename C, typename Observer, typename OffsetPtr>
    struct SerializableContainerImpl;

    template <typename C, typename Observer, typename OffsetPtr, typename... Configs>
    struct Operations<SerializableContainerImpl<C, Observer, OffsetPtr>, Configs...> : ContainerOperations<SerializableContainerImpl<C, Observer, OffsetPtr>, Configs...> {

        static void setActive(SerializableContainerImpl<C, Observer, OffsetPtr> &c, bool active, bool existenceChange)
        {
            c.setActive(active, existenceChange, ContainerOperations<SerializableContainerImpl<C, Observer, OffsetPtr>, Configs...>::Creator::controlled);
        }

        static void setSynced(SerializableContainerImpl<C, Observer, OffsetPtr> &c, bool synced)
        {
            c.setSynced(synced);
        }
    };

    template <typename C, typename Observer, typename OffsetPtr>
    struct SyncableContainerImpl;

    template <typename InnerC, typename Observer, typename OffsetPtr, typename... Configs>
    struct Operations<SyncableContainerImpl<InnerC, Observer, OffsetPtr>, Configs...> : ContainerOperations<SyncableContainerImpl<InnerC, Observer, OffsetPtr>, Configs...> {

        using Base = ContainerOperations<SyncableContainerImpl<InnerC, Observer, OffsetPtr>, Configs...>;

        using C = SyncableContainerImpl<InnerC, Observer, OffsetPtr>;

        using action_payload = typename C::action_payload;
        using request_payload = typename C::request_payload;

        using RequestPolicy = RequestPolicySelector<Configs...>;
        using Creator = CreatorSelector<Configs...>;

        static void setActive(C &c, bool active, bool existenceChange)
        {
            c.setActive(active, existenceChange, Creator::controlled);
        }

        static void setSynced(C &c, bool synced)
        {
            c.setSynced(synced);
        }

        static StreamResult performOperation(C &c, ContainerEvent op, FormattedSerializeStream &in, std::ranges::iterator_t<C> &it, ParticipantId answerTarget, MessageId answerId, const CallerHierarchyBasePtr &hierarchy = {})
        {
            it = c.end();
            switch (op) {
            case EMPLACE: {
                if constexpr (!container_traits<C>::sorted) {
                    STREAM_PROPAGATE_ERROR(Base::readIterator(in, c, it));
                }
                decltype(auto) op = insertOperation(c, it, answerTarget, answerId);
                return TupleUnpacker::invoke(&Creator::template readItem<decltype(op)>, in, op, it, it, hierarchy);
            }
            case ERASE:
                STREAM_PROPAGATE_ERROR(Base::readIterator(in, c, it));
                it = removeOperation(c, it, answerTarget, answerId).erase(it);
                return {};
                /*case REMOVE_RANGE: {
                iterator from = this->read_iterator(in);
                iterator to = this->read_iterator(in);
                it = erase_impl(from, to, answerTarget, answerId);
                break;
            }*/
            case RESET:
                return Base::read(in, c, "content", hierarchy);
            default:
                throw 0;
            }
        }

        //TODO: Maybe move loop out of this function
        static void writeAction(const C &c, const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, action_payload &&payload, const CallerHierarchyBasePtr &hierarchy = {})
        {
            for (FormattedBufferedStream &out : outStreams) {
                std::visit(overloaded {
                               [&](typename C::emplace_action_t &&emplace) {
                                   Serialize::write(out, EMPLACE, "operation");
                                   if constexpr (!container_traits<C>::sorted) {
                                       Base::writeIterator(out, c, emplace.mIt);
                                   }
                                   TupleUnpacker::invoke(&Creator::template writeItem<C>, out, *emplace.mIt, hierarchy);
                               },
                               [&](typename C::erase_t &&erase) {
                                   Serialize::write(out, ERASE, "operation");
                                   Base::writeIterator(out, c, erase.mWhere);
                               },
                               [&](typename C::erase_range_t &&erase) {
                                   Serialize::write(out, ERASE_RANGE, "operation");
                                   Base::writeIterator(out, c, erase.mFrom);
                                   Base::writeIterator(out, c, erase.mTo);
                               },
                               [&](typename C::reset_t &&reset) {
                                   Serialize::write(out, RESET, "operation");
                                   Base::write(out, c, "content", hierarchy);
                               } },
                    std::move(payload));
            }
        }

        static StreamResult readAction(C &c, FormattedSerializeStream &in, PendingRequest &request, const CallerHierarchyBasePtr &hierarchy = {})
        {
            ContainerEvent op;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, op, "operation"));

            bool accepted = (op & ~MASK) != ABORTED;

            if (accepted) {
                std::ranges::iterator_t<C> it;
                STREAM_PROPAGATE_ERROR(performOperation(c, op, in, it, request.mRequester, request.mRequesterTransactionId, hierarchy));
                request.mReceiver.set_value(it);
            } else {
                if (request.mRequesterTransactionId) {
                    FormattedBufferedStream &out = c.getRequestResponseTarget(request.mRequester, request.mRequesterTransactionId);
                    Serialize::write(out, op, "operation");
                    out.endMessageWrite();
                }
                request.mReceiver.set_error(MessageResult::REJECTED);
            }
            return {};
        }

        static void writeRequest(const C &c, FormattedBufferedStream &out, request_payload &&payload, const CallerHierarchyBasePtr &hierarchy = {})
        {
            if (RequestPolicy::sCallByMasterOnly)
                throw 0;

            std::visit(overloaded {
                           [&](typename C::emplace_request_t &&emplace) {
                               Serialize::write(out, EMPLACE, "operation");
                               if constexpr (!container_traits<C>::sorted) {
                                   Base::writeIterator(out, c, emplace.mWhere);
                               }
                               TupleUnpacker::invoke(&Creator::template writeItem<C>, out, emplace.mDummy, hierarchy);
                           },
                           [&](typename C::erase_t &&erase) {
                               Serialize::write(out, ERASE, "operation");
                               Base::writeIterator(out, c, erase.mWhere);
                           },
                           [&](typename C::erase_range_t &&erase) {
                               Serialize::write(out, ERASE_RANGE, "operation");
                               Base::writeIterator(out, c, erase.mFrom);
                               Base::writeIterator(out, c, erase.mTo);
                           },
                           [&](typename C::reset_t &&reset) {
                               Serialize::write(out, RESET, "operation");
                               Base::write(out, c, "content", hierarchy);
                           },
                           [&](typename C::reset_to_request_t &&reset) {
                               Serialize::write(out, RESET, "operation");
                               //Base::write(out, reset.mNewData, "content", hierarchy);
                               throw "TODO";
                           } },
                std::move(payload));
        }

        static StreamResult readRequest(C &c, FormattedBufferedStream &inout, MessageId id, const CallerHierarchyBasePtr &hierarchy = {})
        {
            bool accepted = !RequestPolicy::sCallByMasterOnly;

            ContainerEvent op;
            STREAM_PROPAGATE_ERROR(Serialize::read(inout, op, "operation"));

            if (!accepted) {
                if (id) {
                    c.beginRequestResponseMessage(inout, id);
                    Serialize::write(inout, op | ABORTED, "operation");
                    inout.endMessageWrite();
                }
            } else {
                if (c.isMaster()) {
                    std::ranges::iterator_t<C> it;
                    STREAM_PROPAGATE_ERROR(performOperation(c, op, inout, it, inout.id(), id, hierarchy));
                } else {
                    FormattedBufferedStream &out = c.getSlaveRequestMessageTarget();
                    Serialize::write(out, op, "operation");
                    out.stream().pipe(inout.stream());
                    out.endMessageWrite(inout.id(), id);
                }
            }
            return {};
        }
    };

}
}