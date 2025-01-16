#include "../metalib.h"

#include "syncmanager.h"

#include "hierarchy/toplevelunit.h"

#include "hierarchy/serializableids.h"

#include "hierarchy/statetransmissionflags.h"

#include "streams/syncstreamdata.h"

#include "operations.h"

#include "Generic/execution/execution.h"

#include "Generic/projections.h"

#include "hierarchy/serializetable.h"

#include "Generic/execution/algorithm.h"

#include "container/container_operations.h"

#include "streams/message_streambuf.h"

#include "streams/readmessage.h"

namespace Engine {
namespace Serialize {

    SyncManager::SyncManager(const std::string &name)
        : SerializeManager(name)
    {
    }

    SyncManager::SyncManager(SyncManager &&other) noexcept
        : SerializeManager(std::move(other))
        , mTopLevelUnits(std::move(other.mTopLevelUnits))
        , mReceivingMasterState(std::exchange(other.mReceivingMasterState, nullptr))
    {
        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            unit->removeManager(&other);
            bool result = unit->addManager(this);
            assert(result);
        }
        other.mTopLevelUnits.clear();
        if (other.mSlaveStream) {
            mSlaveStream.emplace(std::move(*other.mSlaveStream), this);
            other.mSlaveStream.reset();
        }
        for (FormattedMessageStream &stream : other.mMasterStreams | std::views::transform(projectionPairSecond)) {
            ParticipantId id = stream.id();
            mMasterStreams.try_emplace(id, std::move(stream), this);
        }
    }

    SyncManager::~SyncManager()
    {
        if (mReceivingMasterState) {
            mReceivingMasterState->set_error(SyncManagerResult::UNKNOWN_ERROR);
            mReceivingMasterState = nullptr;
        }
        clearTopLevelItems();
    }

    void SyncManager::writeHeader(WriteMessage &msg, const SyncableUnitBase *unit, MessageType type)
    {
        msg.beginHeaderWrite();
        write(msg, SerializeManager::convertPtr(static_cast<FormattedSerializeStream&>(msg).stream(), unit), "Object");
        write(msg, type, "MessageType");
        msg.endHeaderWrite();
    }

    void SyncManager::writeActionHeader(WriteMessage &stream, const SyncableUnitBase *unit, MessageType type, MessageId id)
    {
        stream.beginHeaderWrite();
        write(stream, SerializeManager::convertPtr(stream.stream(), unit), "Object");
        write(stream, type, "MessageType");
        write(stream, id, "TransactionId");
        stream.endHeaderWrite();
    }

    StreamResult SyncManager::readMessage(ReadMessage &msg, FormattedMessageStream &stream)
    {
        STREAM_PROPAGATE_ERROR(msg.beginHeaderRead());
        UnitId objectId;
        STREAM_PROPAGATE_ERROR(readState(stream, objectId, "Object"));

        if (objectId == SERIALIZE_MANAGER) {
            ParticipantId id;
            Command cmd;
            STREAM_PROPAGATE_ERROR(readState(stream, cmd, "Command"));
            STREAM_PROPAGATE_ERROR(msg.endHeaderRead());
            switch (cmd) {
            case SET_ID:
                assert(mSlaveStream && &stream == &*mSlaveStream);
                STREAM_PROPAGATE_ERROR(readState(stream, id, "Id"));
                stream.setId(id);
                break;
            case SEND_NAME_MAPPINGS: {
                std::vector<std::pair<std::string, UnitId>> ids;
                STREAM_PROPAGATE_ERROR(readState(stream, ids, "Mappings"));
                for (const auto &[name, id] : ids) {
                    mTopLevelUnitNameMappings.at(name)->setStaticSlaveId(id);
                }
                break;
            }
            default:
                return STREAM_INTEGRITY_ERROR(stream) << "Invalid command used in message header: " << cmd;
            }
        } else {
            SyncableUnitBase *object;
            STREAM_PROPAGATE_ERROR(convertPtr(stream, objectId, object));
            MessageType type;
            STREAM_PROPAGATE_ERROR(readState(stream, type, "MessageType"));
            MessageId transactionId;
            if (type == MessageType::ACTION || type == MessageType::ERROR || type == MessageType::FUNCTION_ACTION || type == MessageType::FUNCTION_ERROR)
                STREAM_PROPAGATE_ERROR(readState(stream, transactionId, "TransactionId"));
            STREAM_PROPAGATE_ERROR(msg.endHeaderRead());

            switch (type) {
            case MessageType::ACTION: {
                PendingRequest request = stream.getRequest(transactionId);
                StreamResult result = object->readAction(stream, request);
                if (result.mState != StreamState::OK) {
                    setError(object, request, MessageResult::DATA_CORRUPTION);
                    return result;
                }
                break;
            }
            case MessageType::REQUEST:
                STREAM_PROPAGATE_ERROR(object->readRequest(stream, msg.mId));
                break;
            case MessageType::STATE:
                assert(object->mType->mIsTopLevelUnit);
                STREAM_PROPAGATE_ERROR(read(stream, *object, "State", {}, StateTransmissionFlags_ApplyMap | StateTransmissionFlags_Activation));
                static_cast<TopLevelUnitBase *>(object)->stateReadDone();
                for (FormattedMessageStream &out : mMasterStreams | std::views::transform(projectionPairSecond)) {
                    sendState(out, object);
                }

                break;
            case MessageType::ERROR: {
                PendingRequest request = stream.getRequest(transactionId);
                setError(object, request, MessageResult::SERVER_ERROR);
                break;
            }
            case MessageType::FUNCTION_ACTION: {
                PendingRequest request = stream.getRequest(transactionId);
                StreamResult result = object->readFunctionAction(stream, request);
                if (result.mState != StreamState::OK) {
                    setError(object, request, MessageResult::DATA_CORRUPTION);
                    return result;
                }
                break;
            }
            case MessageType::FUNCTION_REQUEST:
                STREAM_PROPAGATE_ERROR(object->readFunctionRequest(stream, msg.mId));
                break;
            case MessageType::FUNCTION_ERROR: {
                PendingRequest request = stream.getRequest(transactionId);
                STREAM_PROPAGATE_ERROR(object->readFunctionError(stream, request));
                break;
            }
            default:
                return STREAM_INTEGRITY_ERROR(stream) << "Invalid Message-Type: " << type;
            }
        }
        return {};
    }

    std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId>
    SyncManager::getMasterMessageTargets()
    {
        std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> result;

        for (FormattedMessageStream &stream : mMasterStreams | std::views::transform(projectionPairSecond)) {
            if (stream)
                result.insert(stream);
        }
        return result;
    }

    void SyncManager::clearTopLevelItems()
    {
        while (!mSlaveMappings.empty()) {
            mSlaveMappings.begin()->second->clearSlaveId(this);
        }
        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            unit->removeManager(this);
        }
        mTopLevelUnits.clear();
        mTopLevelUnitNameMappings.clear();
    }

    void SyncManager::addTopLevelItemImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, TopLevelUnitBase *unit, std::string_view name)
    {
        auto pib = mTopLevelUnitNameMappings.try_emplace(std::string { name }, unit);
        assert(pib.second);

        if (!mSlaveStream) {
            for (FormattedMessageStream &stream : mMasterStreams | std::views::transform(projectionPairSecond)) {
                auto msg = stream.beginMessageWrite();
                msg.beginHeaderWrite();
                write<UnitId>(msg, SERIALIZE_MANAGER, "Object");
                write(msg, SEND_NAME_MAPPINGS, "Command");
                msg.endHeaderWrite();
                std::vector<std::pair<std::string_view, UnitId>> ids {
                    { name, unit->masterId() }
                };
                write(msg, ids, "Mappings");
            }
        }

        addTopLevelItemImpl(receiver, unit);
    }

    void SyncManager::addTopLevelItemImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, TopLevelUnitBase *unit, UnitId slaveId)
    {
        if (!unit->addManager(this)) {
            receiver.set_error(SyncManagerResult::UNKNOWN_ERROR);
            return;
        }
        unit->mStaticSlaveId = slaveId;
        mTopLevelUnits.insert(unit);

        if (unit->mSynced) {
            if (mSlaveStream) {
                unit->receiveState(receiver, this);
            } else {
                for (FormattedMessageStream &stream : mMasterStreams | std::views::transform(projectionPairSecond)) {
                    this->sendState(stream, unit);
                }
            }
        }
        receiver.set_value();
    }

    void SyncManager::removeTopLevelItem(TopLevelUnitBase *unit)
    {
        auto it = std::ranges::find(mTopLevelUnitNameMappings, unit, &std::pair<const std::string, TopLevelUnitBase *>::second);
        if (it != mTopLevelUnitNameMappings.end())
            mTopLevelUnitNameMappings.erase(it);

        auto it2 = mSlaveMappings.begin();
        while (it2 != mSlaveMappings.end()) {
            if (it2->second->mTopLevel == unit) {
                it2++->second->clearSlaveId(this);
            } else {
                ++it2;
            }
        }
        unit->removeManager(this);

        mTopLevelUnits.erase(unit);
    }

    void SyncManager::moveTopLevelItem(TopLevelUnitBase *oldUnit,
        TopLevelUnitBase *newUnit)
    {
        removeTopLevelItem(oldUnit);
        addTopLevelItem(newUnit, false);
    }

    FormattedMessageStream &SyncManager::getSlaveMessageTarget()
    {
        assert(mSlaveStream);
        return *mSlaveStream;
    }

    void SyncManager::removeAllStreams()
    {
        removeSlaveStream();
        for (auto it = mMasterStreams.begin(); it != mMasterStreams.end();)
            it = removeMasterStream(it);
    }

    void SyncManager::setSlaveStreamImpl(Execution::VirtualReceiverBase<SyncManagerResult> &receiver, Format format, std::unique_ptr<message_streambuf> buffer,
        TimeOut timeout, std::unique_ptr<SyncStreamData> data)
    {
        if (mSlaveStream) {
            receiver.set_error(SyncManagerResult::UNKNOWN_ERROR);
            return;
        }

        SyncManagerResult state = SyncManagerResult::SUCCESS;

        std::vector<TopLevelUnitBase *> updatedUnits;

        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            if (unit->updateManagerType(this, false)) {
                updatedUnits.push_back(unit);
            } else {
                state = SyncManagerResult::UNKNOWN_ERROR;
                break;
            }
        }

        if (state == SyncManagerResult::SUCCESS) {
            if (!data)
                data = createStreamData();
            mSlaveStream.emplace(format(), std::move(buffer), std::move(data));
            setSlaveStreamData(mSlaveStream->data());

            assert(!mReceivingMasterState);

            mReceivingCounter = 1 + mTopLevelUnits.size();
            mReceivingMasterState = &receiver;
            mReceivingMasterStateTimeout = timeout;
            for (TopLevelUnitBase *unit : mTopLevelUnits) {
                Execution::detach(unit->receiveState(this) | Execution::then([this]() { decreaseReceivingCounter(); }));
            }
            decreaseReceivingCounter();
        } else {
            for (TopLevelUnitBase *unit : updatedUnits | std::views::reverse) {
                bool result = unit->updateManagerType(this, true);
                assert(result);
            }
            receiver.set_error(state);
        }
    }

    void SyncManager::decreaseReceivingCounter()
    {
        assert(mReceivingMasterState);
        if (--mReceivingCounter == 0) {
            Execution::VirtualReceiverBase<SyncManagerResult> *rec = std::exchange(mReceivingMasterState, nullptr);
            rec->set_value();
        }
    }

    void SyncManager::removeSlaveStream(SyncManagerResult reason)
    {
        if (mSlaveStream) {
            while (!mSlaveMappings.empty()) {
                size_t s = mSlaveMappings.size();
                mSlaveMappings.begin()->second->clearSlaveId(this);
                assert(s > mSlaveMappings.size());
            }
            for (TopLevelUnitBase *topLevel : mTopLevelUnits) {
                topLevel->updateManagerType(this, true);
            }
            mSlaveStream->setId(0);
            mSlaveStream.reset();
            setSlaveStreamData(nullptr);
        }

        if (mReceivingMasterState) {
            mReceivingMasterState->set_error(reason);
            mReceivingMasterState = nullptr;
        }
    }

    SyncManagerResult SyncManager::addMasterStream(Format format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data)
    {
        if (!data)
            data = createStreamData();
        ParticipantId id = data->id();
        auto pib = mMasterStreams.try_emplace(id, format(), std::move(buffer), std::move(data));

        assert(pib.second);
        
        FormattedMessageStream &stream = pib.first->second;

        {
            auto msg = stream.beginMessageWrite();
            msg.beginHeaderWrite();
            write<UnitId>(msg, SERIALIZE_MANAGER, "Object");
            write(msg, SET_ID, "Command");
            msg.endHeaderWrite();
            write(msg, stream.id(), "Id");
        }

        {
            auto msg = stream.beginMessageWrite();
            msg.beginHeaderWrite();
            write<UnitId>(msg, SERIALIZE_MANAGER, "Object");
            write(msg, SEND_NAME_MAPPINGS, "Command");
            msg.endHeaderWrite();
            std::vector<std::pair<std::string_view, UnitId>> ids;
            std::ranges::transform(mTopLevelUnitNameMappings, std::back_inserter(ids), [](const std::pair<const std::string, TopLevelUnitBase *> &p) {
                return std::pair<std::string_view, UnitId> { p.first, p.second->masterId() };
            });
            write(msg, ids, "Mappings");
        }

        for (TopLevelUnitBase *unit : mTopLevelUnits) {
            sendState(stream, unit);
        }

        
        return SyncManagerResult::SUCCESS;
    }

    SyncManagerResult SyncManager::moveMasterStream(ParticipantId streamId,
        SyncManager *target)
    {
        auto it = mMasterStreams.find(streamId);
        assert(it != mMasterStreams.end());
        auto pib = target->mMasterStreams.try_emplace(streamId, std::move(it->second));

        FormattedMessageStream &stream = pib.first->second;
        std::vector<TopLevelUnitBase *> newTopLevels;
        newTopLevels.reserve(16);
        set_difference(target->getTopLevelUnits().begin(),
            target->getTopLevelUnits().end(), getTopLevelUnits().begin(),
            getTopLevelUnits().end(), back_inserter(newTopLevels));
        for (TopLevelUnitBase *newTopLevel : newTopLevels) {
            sendState(stream, newTopLevel);
        }

        return SyncManagerResult::SUCCESS;
    }

    std::map<ParticipantId, FormattedMessageStream>::iterator SyncManager::removeMasterStream(std::map<ParticipantId, FormattedMessageStream>::iterator it, SyncManagerResult reason)
    {
        return mMasterStreams.erase(it);
    }

    ParticipantId SyncManager::getParticipantId(SyncManager *manager)
    {
        if (manager && manager->mSlaveStream) {
            return manager->mSlaveStream->id();
        } else {
            return sLocalMasterParticipantId;
        }
    }

    void SyncManager::setError(SyncableUnitBase *unit, PendingRequest &pending, MessageResult error)
    {
        if (pending.mReceiver)
            pending.mReceiver.set_error(error);
        if (pending.mRequester) {
            auto it = mMasterStreams.find(pending.mRequester);
            assert(it != mMasterStreams.end());
            FormattedMessageStream &stream = it->second;
            auto msg = stream.beginMessageWrite();
            writeActionHeader(msg, unit, MessageType::ERROR, pending.mRequesterTransactionId);            
        }
    }

    void SyncManager::receiveMessages(int msgCount, TimeOut timeout)
    {
        if (mSlaveStream) {
            StreamResult result = receiveMessages(*mSlaveStream, msgCount, timeout);
            switch (result.mState) {
            case StreamState::OK:
                if (mReceivingMasterState && mReceivingMasterStateTimeout.expired()) {
                    //StreamResult result = STREAM_INTEGRITY_ERROR(*mSlaveStream) << "Server did not provide initial state in time (timeout)";
                    removeSlaveStream(SyncManagerResult::TIMEOUT);
                }
                break;
            default:
                removeSlaveStream();
            }
        }
        for (auto it = mMasterStreams.begin(); it != mMasterStreams.end();) {
            StreamResult result = receiveMessages(it->second, msgCount, timeout);
            switch (result.mState) {
            case StreamState::OK:
                ++it;
                break;
            default:
                it = removeMasterStream(it);
            }
        }
    }

    void SyncManager::sendMessages()
    {
        if (mSlaveStream) {
            StreamResult result = mSlaveStream->sendMessages();
            switch (result.mState) {
            case StreamState::OK:
                break;
            default:
                removeSlaveStream();
            }
        }
        for (auto it = mMasterStreams.begin(); it != mMasterStreams.end();) {
            StreamResult result = it->second.sendMessages();
            switch (result.mState) {
            case StreamState::OK:
                ++it;
                break;
            default:
                it = removeMasterStream(it);
            }
        }
    }

    void SyncManager::sendAndReceiveMessages()
    {
        receiveMessages();
        sendMessages();
    }

    StreamResult SyncManager::convertPtr(FormattedSerializeStream &in,
        UnitId unit, SyncableUnitBase *&out)
    {
        if (unit == NULL_UNIT_ID) {
            out = nullptr;
            return {};
        }
        try {
            if (mSlaveStream && (&in == &*mSlaveStream)) {
                out = mSlaveMappings.at(unit);
            } else {
                if (unit < RESERVED_ID_COUNT) {
                    assert(unit >= BEGIN_USER_ID_SPACE);
                    auto it = std::ranges::find(
                        mTopLevelUnits, unit, &TopLevelUnitBase::masterId);
                    if (it == mTopLevelUnits.end()) {
                        return STREAM_INTEGRITY_ERROR(in) << "Illegal TopLevel-Id (" << unit << ") used!";
                    }
                    out = *it;
                } else {
                    SyncableUnitBase *u = getByMasterId(unit);
                    if (!u)
                        return STREAM_INTEGRITY_ERROR(in) << "Non-existing Unit-Id (" << unit << ") used!";
                    if (std::ranges::find(mTopLevelUnits, u->mTopLevel) == mTopLevelUnits.end())
                        return STREAM_INTEGRITY_ERROR(in) << "Unit (" << unit << ") with unregistered TopLevel-Unit used!";

                    out = u;
                }
            }
        } catch (const std::out_of_range &) {
            return STREAM_INTEGRITY_ERROR(in.stream(), true) << "Unknown Syncable Unit-Id (" << unit << ") used!";
        }
        return {};
    }

    std::vector<ParticipantId> SyncManager::getMasterParticipantIds()
    {
        std::vector<ParticipantId> result;
        result.reserve(mMasterStreams.size());
        for (const FormattedMessageStream &stream : mMasterStreams | std::views::transform(projectionPairSecond)) {
            result.push_back(stream.id());
        }
        return result;
    }

    StreamResult SyncManager::receiveMessages(FormattedMessageStream &stream,
        int &msgCount, TimeOut timeout)
    {
        ReadMessage msg;
        if (msgCount >= 0) {
            while (stream && msgCount > 0) {
                STREAM_PROPAGATE_ERROR(stream.beginMessageRead(msg));
                while (msg) {
                    STREAM_PROPAGATE_ERROR(readMessage(msg, stream));
                    STREAM_PROPAGATE_ERROR(msg.end());
                    --msgCount;
                    if (!timeout.isZero() && timeout.expired())
                        break;
                    STREAM_PROPAGATE_ERROR(stream.beginMessageRead(msg));
                }
                if (timeout.expired())
                    break;
            }
        } else {
            STREAM_PROPAGATE_ERROR(stream.beginMessageRead(msg));
            while (msg) {
                STREAM_PROPAGATE_ERROR(readMessage(msg, stream));
                STREAM_PROPAGATE_ERROR(msg.end());
                if (!timeout.isZero() && timeout.expired())
                    break;
                STREAM_PROPAGATE_ERROR(stream.beginMessageRead(msg));
            }
        }

        return {};
    }

    FormattedMessageStream *SyncManager::getSlaveStream()
    {
        return mSlaveStream ? &*mSlaveStream : nullptr;
    }

    FormattedMessageStream &SyncManager::getMasterStream(ParticipantId id)
    {
        return mMasterStreams.find(id)->second;
    }

    const std::set<TopLevelUnitBase *> &
    SyncManager::getTopLevelUnits() const
    {
        return mTopLevelUnits;
    }

    size_t SyncManager::clientCount() const
    {
        return mMasterStreams.size();
    }

    void SyncManager::sendState(FormattedMessageStream &stream,
        SyncableUnitBase *unit)
    {
        auto msg = stream.beginMessageWrite();
        msg.beginHeaderWrite();
        write(stream, SerializeManager::convertPtr(stream.stream(), unit), "Object");
        write<MessageType>(stream, MessageType::STATE, "MessageType");
        msg.endHeaderWrite();
        write(stream, *unit, "State");
    }

    std::unique_ptr<SyncStreamData> SyncManager::createStreamData(ParticipantId id)
    {
        return std::make_unique<SyncStreamData>(*this, id);
    }

} // namespace Serialize
} // namespace Engine
