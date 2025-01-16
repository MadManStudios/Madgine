#pragma once

#include "serializemanager.h"

#include "Generic/genericresult.h"

#include "Generic/timeout.h"

#include "Generic/container/mutable_set.h"

#include "streams/comparestreamid.h"

#include "streams/formattedmessagestream.h"

#include "Generic/execution/virtualsender.h"

#include "Generic/functor.h"

namespace Engine {
namespace Serialize {

    ENUM_BASE(SyncManagerResult, GenericResult,
        STREAM_ERROR,
        TIMEOUT,
        CANCELED)

    struct META_EXPORT SyncManager : SerializeManager {
        SyncManager(const std::string &name);
        SyncManager(const SerializeManager &) = delete;
        SyncManager(SyncManager &&) noexcept;
        ~SyncManager();

        void clearTopLevelItems();
        void addTopLevelItemImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, TopLevelUnitBase *unit, std::string_view name);
        void addTopLevelItemImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, TopLevelUnitBase *unit, UnitId slaveId = 0);
        ASYNC_STUB(addTopLevelItem, addTopLevelItemImpl, Execution::make_simple_virtual_sender<SyncManagerResult>);
        void removeTopLevelItem(TopLevelUnitBase *unit);
        void moveTopLevelItem(TopLevelUnitBase *oldUnit, TopLevelUnitBase *newUnit);

        std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> getMasterMessageTargets();

        FormattedMessageStream &getSlaveMessageTarget();

        std::vector<ParticipantId> getMasterParticipantIds();
        size_t clientCount() const;

        static void writeHeader(WriteMessage &msg, const SyncableUnitBase *unit, MessageType type);
        static void writeActionHeader(WriteMessage &msg, const SyncableUnitBase *unit, MessageType type, MessageId id);
        StreamResult readMessage(ReadMessage &msg, FormattedMessageStream &stream);

        void receiveMessages(int msgCount = -1, TimeOut timeout = {});
        void sendMessages();
        void sendAndReceiveMessages();

        StreamResult convertPtr(FormattedSerializeStream &in, UnitId unit, SyncableUnitBase *&out);

        static ParticipantId getParticipantId(SyncManager *manager);

    protected:
        StreamResult receiveMessages(FormattedMessageStream &stream, int &msgCount, TimeOut timeout = {});

        FormattedMessageStream *getSlaveStream();
        FormattedMessageStream &getMasterStream(ParticipantId id);

        void removeAllStreams();
        void setSlaveStreamImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, Format format, std::unique_ptr<message_streambuf> buffer, TimeOut timeout = {}, std::unique_ptr<SyncStreamData> data = {});
        ASYNC_STUB(setSlaveStream, setSlaveStreamImpl, Execution::make_simple_virtual_sender<SyncManagerResult>);
        void decreaseReceivingCounter();
        virtual void removeSlaveStream(SyncManagerResult reason = SyncManagerResult::UNKNOWN_ERROR);

        SyncManagerResult addMasterStream(Format format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data = {});
        SyncManagerResult moveMasterStream(ParticipantId streamId, SyncManager *target);
        virtual std::map<ParticipantId, FormattedMessageStream>::iterator removeMasterStream(std::map<ParticipantId, FormattedMessageStream>::iterator it, SyncManagerResult reason = SyncManagerResult::UNKNOWN_ERROR);

        const std::set<TopLevelUnitBase *> &getTopLevelUnits() const;

        void sendState(FormattedMessageStream &stream, SyncableUnitBase *unit);

        void setError(SyncableUnitBase *unit, PendingRequest &pending, MessageResult error);

        std::unique_ptr<SyncStreamData> createStreamData(ParticipantId id = createStreamId());

        Execution::VirtualReceiverBase<SyncManagerResult> *mReceivingMasterState = nullptr;
        TimeOut mReceivingMasterStateTimeout;
        size_t mReceivingCounter;

    private:
        std::map<ParticipantId, FormattedMessageStream> mMasterStreams;
        std::optional<FormattedMessageStream> mSlaveStream;

        std::set<TopLevelUnitBase *> mTopLevelUnits; //TODO: Sort by MasterId

        std::map<std::string, TopLevelUnitBase *> mTopLevelUnitNameMappings;        
    };
}
}
