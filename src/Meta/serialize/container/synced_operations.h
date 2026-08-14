#pragma once

namespace Engine {
namespace Serialize {

    DERIVE_OPERATOR(PlusAssign, +=)
    DERIVE_OPERATOR(MinusAssign, -=)

    template <typename T, typename Observer, typename OffsetPtr>
    struct Operations<Synced<T, Observer, OffsetPtr>> {
        template <typename Context>
        static StreamResult readRequest(Synced<T, Observer, OffsetPtr> &synced, FormattedMessageStream &inout, MessageId id, Context &&context)
        {
            if (synced.isMaster()) {
                typename Synced<T, Observer, OffsetPtr>::Operation op;
                STREAM_PROPAGATE_ERROR(Serialize::read(inout, op, nullptr));
                T old = synced.mData;
                T value;
                STREAM_PROPAGATE_ERROR(Serialize::read(inout, value, nullptr));
                switch (op) {
                case Synced<T, Observer, OffsetPtr>::Operation::SET:
                    synced.mData = value;
                    break;
                case Synced<T, Observer, OffsetPtr>::Operation::ADD:
                    if constexpr (has_operator_PlusAssign<T, T>)
                        synced.mData += value;
                    else
                        throw 0;
                    break;
                case Synced<T, Observer, OffsetPtr>::Operation::SUB:
                    if constexpr (has_operator_MinusAssign<T, T>)
                        synced.mData -= value;
                    else
                        throw 0;
                    break;
                }
                synced.notify(old, inout.id(), id);
            } else {
                WriteMessage msg = getSlaveRequestMessageTarget(&synced, inout.id(), id);
                msg.stream().pipe(inout.stream());
            }
            return {};
        }

        template <typename Context>
        static void writeRequest(const Synced<T, Observer, OffsetPtr> &synced, FormattedMessageStream &out, Synced<T, Observer, OffsetPtr>::request_payload &&payload, Context &&context)
        {
            Serialize::write(out, payload.mOperation, nullptr, context);
            Serialize::write(out, payload.mValue, nullptr, context);
        }

        template <typename Context>
        static StreamResult readAction(Synced<T, Observer, OffsetPtr> &synced, FormattedMessageStream &in, PendingRequest &request, Context &&context)
        {
            typename Synced<T, Observer, OffsetPtr>::Operation op;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, op, nullptr));
            T old = synced.mData;
            T value;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, value, nullptr));
            switch (op) {
            case Synced<T, Observer, OffsetPtr>::Operation::SET:
                synced.mData = value;
                break;
            case Synced<T, Observer, OffsetPtr>::Operation::ADD:
                if constexpr (has_operator_PlusAssign<T, T>)
                    synced.mData += value;
                else
                    throw 0;
                break;
            case Synced<T, Observer, OffsetPtr>::Operation::SUB:
                if constexpr (has_operator_MinusAssign<T, T>)
                    synced.mData -= value;
                else
                    throw 0;
                break;
            }
            synced.notify(old);
            return {};
        }

        template <typename Context>
        static void writeAction(const Synced<T, Observer, OffsetPtr> &synced, const std::vector<WriteMessage> &outStreams, Synced<T, Observer, OffsetPtr>::action_payload &&payload, Context &&context)
        {
            for (const WriteMessage &out : outStreams) {
                Serialize::write(out, payload.mOperation, nullptr, context);
                Serialize::write(out, payload.mValue, nullptr, context);
            }
        }

        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, Synced<T, Observer, OffsetPtr> &synced, const char *name, Context &&context)
        {
            T old = synced.mData;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, synced.mData, name, context));
            synced.notify(old);
            return {};
        }

        template <typename Context>
        static void write(FormattedSerializeStream &out, const Synced<T, Observer, OffsetPtr> &synced, const char *name, Context &&context)
        {
            Serialize::write(out, synced.mData, name, context);
        }

        static void setActive(Synced<T, Observer, OffsetPtr> &synced, bool active, bool existenceChanged)
        {
            if (!active) {
                if (synced.mData != T {})
                    synced.Observer::operator()(T {}, synced.mData);
            }
            Serialize::setActive(synced.mData, active, existenceChanged);
            if (active) {
                if (synced.mData != T {})
                    synced.Observer::operator()(synced.mData, T {});
            }
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            return Serialize::visitStream<T>(in, name, visitor);
        }
    };

}
}