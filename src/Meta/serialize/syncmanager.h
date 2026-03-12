#pragma once

#include "Generic/execution/virtualsender.h"
#include "Generic/functor.h"
#include "Generic/genericresult.h"
#include "Generic/timeout.h"

#include "serializemanager.h"
#include "streams/comparestreamid.h"
#include "streams/formattedmessagestream.h"

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
        Execution::Future<SyncManagerResult> addTopLevelItem(TopLevelUnitBase *unit, std::string_view name);
        Execution::Future<SyncManagerResult> addTopLevelItem(TopLevelUnitBase *unit, UnitId slaveId = 0);
        void removeTopLevelItem(TopLevelUnitBase *unit);
        Engine::Execution::Future<SyncManagerResult> moveTopLevelItem(TopLevelUnitBase *oldUnit, TopLevelUnitBase *newUnit);

        std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> getMasterMessageTargets();

        FormattedMessageStream &getSlaveMessageTarget();

        std::set<ParticipantId> clients();
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
        Execution::Future<SyncManagerResult> setSlaveStream(Format format, std::unique_ptr<message_streambuf> buffer, TimeOut timeout = {}, std::unique_ptr<SyncStreamData> data = {});
        void decreaseReceivingCounter();
        virtual void removeSlaveStream(SyncManagerResult reason = SyncManagerResult::UNKNOWN_ERROR);

        SyncManagerResult addMasterStream(Format format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data = {});
        SyncManagerResult moveMasterStream(ParticipantId streamId, SyncManager *target);
        virtual std::map<ParticipantId, FormattedMessageStream>::iterator removeMasterStream(std::map<ParticipantId, FormattedMessageStream>::iterator it, SyncManagerResult reason = SyncManagerResult::UNKNOWN_ERROR);

        const std::set<TopLevelUnitBase *> &getTopLevelUnits() const;

        void sendState(FormattedMessageStream &stream, SyncableUnitBase *unit);

        void setError(SyncableUnitBase *unit, PendingRequest &pending, MessageResult error);

        std::unique_ptr<SyncStreamData> createStreamData(ParticipantId id = createStreamId());

        Execution::Promise<SyncManagerResult> mReceivingMasterState = std::nullopt;
        TimeOut mReceivingMasterStateTimeout;
        size_t mReceivingCounter;

    private:
        std::map<ParticipantId, FormattedMessageStream> mMasterStreams;
        std::optional<FormattedMessageStream> mSlaveStream;

        std::set<TopLevelUnitBase *> mTopLevelUnits; // TODO: Sort by MasterId

        std::map<std::string, TopLevelUnitBase *> mTopLevelUnitNameMappings;
    };
}
}
