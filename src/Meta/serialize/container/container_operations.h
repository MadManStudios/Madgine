#pragma once

#include "Generic/containers/containerevent.h"

#include "../configs/creator.h"
#include "../configs/filter.h"
#include "../configs/requestpolicy.h"
#include "../operations.h"
#include "../primitivetypes.h"
#include "../streams/comparestreamid.h"
#include "../streams/pendingrequest.h"
#include "../streams/writemessage.h"

namespace Engine {
namespace Serialize {

    template <typename C, typename... Configs>
    struct ContainerOperations {

        using T = typename C::value_type;

        using Creator = CreatorSelector<Configs...>;
        using Filter = FilterSelector<Configs...>;

        template <typename Op>
        static StreamResult readOp(CallerHierarchyFormattedSerializeStream in, Op &op, const char *name)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginContainerRead(name, !Containers::container_traits<C>::is_fixed_size));

            if constexpr (!Containers::container_traits<C>::is_fixed_size) {
                TupleUnpacker::invoke(&Creator::template clear<Op>, op, in.mHierarchy);

                while (in.mStream.hasContainerItem()) {
                    std::ranges::iterator_t<Op> it;
                    STREAM_PROPAGATE_ERROR(TupleUnpacker::invoke(&Creator::template readItem<Op>, in, op, it, physical(op).end()));
                }
            } else {
                for (T &t : physical(op)) {
                    if (Filter::filter(t))
                        STREAM_PROPAGATE_ERROR(Serialize::read(in, t, "Item"));
                }
            }

            return in.mStream.endContainerRead(name);
        }

        static StreamResult read(CallerHierarchyFormattedSerializeStream in, C &container, const char *name)
        {
            decltype(auto) op = Containers::resetOperation(container, Creator::controlled);
            return readOp(in, op, name);
        }

        static void write(CallerHierarchyFormattedSerializeStream out, const C &container, const char *name)
        {
            if constexpr (Containers::container_traits<C>::is_fixed_size)
                out.mStream.beginContainerWrite(name);
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
                out.mStream.beginContainerWrite(name, size);
            }
            for (const auto &t : physical(container)) {
                if (Filter::filter(t))
                    TupleUnpacker::invoke(&Creator::template writeItem<C>, out, t);
            }
            out.mStream.endContainerWrite(name);
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            STREAM_PROPAGATE_ERROR(in.mStream.beginContainerRead(name, !Containers::container_traits<C>::is_fixed_size));

            while (in.mStream.hasContainerItem()) {
                STREAM_PROPAGATE_ERROR(Creator::template visitStream<C>(in, visitor, depth + 1));
            }

            return in.mStream.endContainerRead(name);
        }

        static StreamResult readIterator(CallerHierarchyFormattedSerializeStream in, C &c, typename C::iterator &it)
        {
            int32_t dist;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, dist, "it"));
            it = std::next(c.begin(), dist);
            return {};
        }

        static void writeIterator(CallerHierarchyFormattedSerializeStream out, const C &c, const typename C::const_iterator &it)
        {
            Serialize::write<int32_t>(out, std::distance(c.begin(), it), "it");
        }
    };

    template <SerializeRange C, typename... Configs>
    struct Operations<C, Configs...> : ContainerOperations<C, Configs...> {
    };

    template <typename C>
    concept SerializeWrappedRange = SerializeRange<C> && requires {
        typename Containers::underlying_container<C>::type;
    };

    template <SerializeWrappedRange C, typename... Configs>
    struct Operations<C, Configs...> : Operations<typename Containers::underlying_container<C>::type, Configs...> {
    };

    template <typename C, typename Observer, typename OffsetPtr>
    struct SerializableContainerImpl;

    template <typename C, typename Observer, typename OffsetPtr, typename... Configs>
    struct Operations<SerializableContainerImpl<C, Observer, OffsetPtr>, Configs...> : ContainerOperations<SerializableContainerImpl<C, Observer, OffsetPtr>, Configs...> {
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

        static StreamResult performOperation(C &c, Containers::ContainerEvent op, CallerHierarchyFormattedSerializeStream in, std::ranges::iterator_t<C> &it, ParticipantId answerTarget, MessageId answerId)
        {
            it = c.end();
            switch (op) {
            case Containers::EMPLACE: {
                if constexpr (!Containers::container_traits<C>::sorted) {
                    STREAM_PROPAGATE_ERROR(Base::readIterator(in, c, it));
                }
                decltype(auto) op = Containers::insertOperation(c, it, answerTarget, answerId);
                return TupleUnpacker::invoke(&Creator::template readItem<decltype(op)>, in, op, it, it);
            }
            case Containers::ERASE:
                STREAM_PROPAGATE_ERROR(Base::readIterator(in, c, it));
                it = Containers::removeOperation(c, it, answerTarget, answerId).erase(it);
                return {};
                /*case REMOVE_RANGE: {
                iterator from = this->read_iterator(in);
                iterator to = this->read_iterator(in);
                it = erase_impl(from, to, answerTarget, answerId);
                break;
            }*/
            case Containers::RESET:
                return Base::read(in, c, "content");
            default:
                throw 0;
            }
        }

        // TODO: Maybe move loop out of this function
        static void writeAction(const C &c, const std::vector<WriteMessage> &outStreams, action_payload &&payload, const CallerHierarchyBasePtr &hierarchy = {})
        {
            for (FormattedMessageStream &out : outStreams) {
                std::visit(overloaded {
                               [&](typename C::emplace_action_t &&emplace) {
                                   Serialize::write(out, Containers::EMPLACE, "operation");
                                   if constexpr (!Containers::container_traits<C>::sorted) {
                                       Base::writeIterator(out, c, emplace.mIt);
                                   }
                                   TupleUnpacker::invoke(&Creator::template writeItem<C>, CallerHierarchyFormattedSerializeStream { out, hierarchy }, *emplace.mIt);
                               },
                               [&](typename C::erase_t &&erase) {
                                   Serialize::write({ out, hierarchy }, Containers::ERASE, "operation");
                                   Base::writeIterator({ out, hierarchy }, c, erase.mWhere);
                               },
                               [&](typename C::erase_range_t &&erase) {
                                   Serialize::write({ out, hierarchy }, Containers::ERASE_RANGE, "operation");
                                   Base::writeIterator({ out, hierarchy }, c, erase.mFrom);
                                   Base::writeIterator({ out, hierarchy }, c, erase.mTo);
                               },
                               [&](typename C::reset_t &&reset) {
                                   Serialize::write({ out, hierarchy }, Containers::RESET, "operation");
                                   Base::write({ out, hierarchy }, c, "content");
                               } },
                    std::move(payload));
            }
        }

        static StreamResult readAction(C &c, CallerHierarchyFormattedSerializeStream in, PendingRequest &request)
        {
            Containers::ContainerEvent op;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, op, "operation"));

            bool accepted = (op & ~Containers::MASK) != Containers::ABORTED;

            if (accepted) {
                std::ranges::iterator_t<C> it;
                STREAM_PROPAGATE_ERROR(performOperation(c, op, in, it, request.mRequester, request.mRequesterTransactionId));
                request.mReceiver.set_value<std::ranges::iterator_t<C>>(it);
            } else {
                if (request.mRequesterTransactionId) {
                    WriteMessage msg = getRequestResponseTarget(&c, request.mRequester, request.mRequesterTransactionId);
                    Serialize::write(msg, op, "operation");
                }
                request.mReceiver.set_error(MessageResult::REJECTED);
            }
            return {};
        }

        static void writeRequest(const C &c, CallerHierarchyFormattedSerializeStream out, request_payload &&payload)
        {
            if (RequestPolicy::sCallByMasterOnly)
                throw 0;

            std::visit(overloaded {
                           [&](typename C::emplace_request_t &&emplace) {
                               Serialize::write(out, Containers::EMPLACE, "operation");
                               if constexpr (!Containers::container_traits<C>::sorted) {
                                   Base::writeIterator(out, c, emplace.mWhere);
                               }
                               TupleUnpacker::invoke(&Creator::template writeItem<C>, out, emplace.mDummy);
                           },
                           [&](typename C::erase_t &&erase) {
                               Serialize::write(out, Containers::ERASE, "operation");
                               Base::writeIterator(out, c, erase.mWhere);
                           },
                           [&](typename C::erase_range_t &&erase) {
                               Serialize::write(out, Containers::ERASE_RANGE, "operation");
                               Base::writeIterator(out, c, erase.mFrom);
                               Base::writeIterator(out, c, erase.mTo);
                           },
                           [&](typename C::reset_t &&reset) {
                               Serialize::write(out, Containers::RESET, "operation");
                               Base::write(out, c, "content");
                           },
                           [&](typename C::reset_to_request_t &&reset) {
                               Serialize::write(out, Containers::RESET, "operation");
                               // Base::write(out, reset.mNewData, "content", hierarchy);
                               throw "TODO";
                           } },
                std::move(payload));
        }

        static StreamResult readRequest(C &c, FormattedMessageStream &inout, MessageId id, const CallerHierarchyBasePtr &hierarchy = {})
        {
            bool accepted = !RequestPolicy::sCallByMasterOnly;

            Containers::ContainerEvent op;
            STREAM_PROPAGATE_ERROR(Serialize::read(inout, op, "operation"));

            if (!accepted) {
                if (id) {
                    auto msg = beginRequestResponseMessage(&c, inout, id);
                    Serialize::write(msg, op | Containers::ABORTED, "operation");
                }
            } else {
                if (c.isMaster()) {
                    std::ranges::iterator_t<C> it;
                    STREAM_PROPAGATE_ERROR(performOperation(c, op, { inout, hierarchy }, it, inout.id(), id));
                } else {
                    WriteMessage out = getSlaveRequestMessageTarget(&c, inout.id(), id);
                    Serialize::write(out, op, "operation");
                    out.stream().pipe(inout.stream());
                }
            }
            return {};
        }
    };

}
}