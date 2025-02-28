#pragma once

#include "../streams/comparestreamid.h"

#include "../streams/pendingrequest.h"

#include "../streams/writemessage.h"

namespace Engine {
namespace Serialize {

    META_EXPORT WriteMessage getSlaveRequestMessageTarget(const SyncableUnitBase *unit, ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver);
    META_EXPORT std::vector<WriteMessage> getMasterActionMessageTargets(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId,
        const std::set<ParticipantId> &targets = {});
    META_EXPORT WriteMessage beginRequestResponseMessage(const SyncableUnitBase *unit, FormattedMessageStream &stream, MessageId id);


    struct META_EXPORT SyncableBase {
    };

    template <typename OffsetPtr>
    struct Syncable : SyncableBase {

        friend std::vector<WriteMessage> getMasterActionMessageTargets(const Syncable<OffsetPtr> *syncable, ParticipantId answerTarget = 0, MessageId answerId = 0,
            const std::set<ParticipantId> &targets = {})
        {
            return getMasterActionMessageTargets(OffsetPtr::parent(syncable), answerTarget, answerId, targets);
        }

        friend WriteMessage getSlaveRequestMessageTarget(const Syncable<OffsetPtr> *syncable, ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver = {})
        {
            return getSlaveRequestMessageTarget(OffsetPtr::parent(syncable), requester, requestId, std::move(receiver));
        }

        ParticipantId participantId()
        {
            return parent()->participantId();
        }

        template <typename... Args>
        void writeAction(ParticipantId answerTarget, MessageId answerId, Args &&...args) const
        {
            parent()->writeAction(static_cast<const typename OffsetPtr::member_type *>(this), answerTarget, answerId, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void writeRequest(ParticipantId requester, MessageId requesterTransactionId, Args &&...args) const
        {
            parent()->writeRequest(static_cast<const typename OffsetPtr::member_type *>(this), requester, requesterTransactionId, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void writeRequest(GenericMessageReceiver receiver, Args &&...args) const
        {
            parent()->writeRequest(static_cast<const typename OffsetPtr::member_type *>(this), std::move(receiver), std::forward<Args>(args)...);
        }

        template <typename... Args>
        void writeRequestResponse(ParticipantId answerTarget, MessageId answerId, Args &&...args) const
        {
            parent()->writeRequestResponse(static_cast<const typename OffsetPtr::member_type *>(this), answerTarget, answerId, std::forward<Args>(args)...);
        }

        friend WriteMessage beginRequestResponseMessage(const Syncable<OffsetPtr> *syncable, FormattedMessageStream &stream, MessageId id)
        {
            return beginRequestResponseMessage(OffsetPtr::parent(syncable), stream, id);
        }

        friend WriteMessage getRequestResponseTarget(const Syncable<OffsetPtr> *syncable, ParticipantId stream, MessageId id)
        {
            return getMasterRequestResponseTarget(OffsetPtr::parent(syncable), stream, id);
        }

        bool isMaster() const
        {
            return parent()->isMaster();
        }

        auto parent() const
        {
            return OffsetPtr::parent(this);
        }

        /*bool isActive() const
        {
                    return !parent() || parent()->isActive(parent()->type()->getIndex(OffsetPtr::template offset<SerializableDataUnit>()));
        }*/
    };
}
}
