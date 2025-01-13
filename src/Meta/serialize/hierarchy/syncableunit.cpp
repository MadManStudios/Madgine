#include "../../metalib.h"

#include "syncableunit.h"

#include "statetransmissionflags.h"

#include "serializetable.h"

#include "toplevelunit.h"

#include "../syncmanager.h"

#include "../operations.h"

namespace Engine {
namespace Serialize {

    SyncableUnitBase::SyncableUnitBase(UnitId masterId)
        : mMasterId(SerializeManager::generateMasterId(masterId, this))
    {
    }

    SyncableUnitBase::SyncableUnitBase(const SyncableUnitBase &other)
        : SerializableUnitBase(other)
        , mMasterId(SerializeManager::generateMasterId(0, this))
    {
    }

    SyncableUnitBase::SyncableUnitBase(SyncableUnitBase &&other) noexcept
        : SerializableUnitBase(std::move(other))
        , mSlaveId(std::exchange(other.mSlaveId, 0))
        , mMasterId(SerializeManager::updateMasterId(std::exchange(other.mMasterId, SerializeManager::generateMasterId(0, &other)), this))
    {
    }

    SyncableUnitBase::~SyncableUnitBase()
    {
        assert(!mSlaveId);
        SerializeManager::deleteMasterId(mMasterId, this);
    }

    SyncableUnitBase &SyncableUnitBase::operator=(const SyncableUnitBase &other)
    {
        SerializableUnitBase::operator=(other);
        return *this;
    }

    SyncableUnitBase &SyncableUnitBase::operator=(SyncableUnitBase &&other)
    {
        SerializableUnitBase::operator=(std::move(other));
        mSlaveId = std::exchange(other.mSlaveId, 0);
        std::swap(mMasterId, other.mMasterId);
        SerializeManager::updateMasterId(mMasterId, this);
        SerializeManager::updateMasterId(other.mMasterId, &other);
        return *this;
    }

    void SyncableUnitBase::writeState(FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy, StateTransmissionFlags flags) const
    {
        if (out.isMaster(AccessMode::WRITE) && out.data() && !(flags & StateTransmissionFlags_SkipId)) {
            out.beginExtendedWrite(name, 1);
            Serialize::writeState(out, mMasterId, "syncId");
        }
        customUnitPtr().writeState(out, name, hierarchy, flags | StateTransmissionFlags_SkipId);
    }

    StreamResult SyncableUnitBase::readState(FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy, StateTransmissionFlags flags)
    {
        if (!in.isMaster(AccessMode::READ) && in.data() && !(flags & StateTransmissionFlags_SkipId)) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            UnitId id;
            STREAM_PROPAGATE_ERROR(Serialize::readState(in, id, "syncId"));

            if (in.manager() && in.manager()->getSlaveStreamData() == in.data()) {
                setSlaveId(id, in.manager());
            }
        }
        return customUnitPtr().readState(in, name, hierarchy, flags | StateTransmissionFlags_SkipId);
    }

    void SyncableUnitBase::setActive(bool active, bool existenceChanged)
    {
        mType->setActive(this, active, existenceChanged);
    }

    StreamResult SyncableUnitBase::visitStream(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        assert(!in.isMaster(AccessMode::READ));
        STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
        UnitId id;
        STREAM_PROPAGATE_ERROR(read(in, id, "syncId"));
        return SerializableDataPtr::visitStream(table, in, name, visitor);
    }

    StreamResult SyncableUnitBase::readAction(FormattedBufferedStream &in, PendingRequest &request)
    {
        return mType->readAction(this, in, request);
    }

    StreamResult SyncableUnitBase::readRequest(FormattedBufferedStream &in, MessageId id)
    {
        return mType->readRequest(this, in, id);
    }

    std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> SyncableUnitBase::getMasterMessageTargets(const std::set<ParticipantId> &targets) const
    {
        std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> result;
        if (mSynced) {
            result = mTopLevel->getMasterMessageTargets();
            if (!targets.empty()) {
                std::erase_if(result,
                    [&](FormattedBufferedStream &stream) { return !targets.contains(stream.id()); });
            }
        }
        return result;
    }

    FormattedBufferedStream &getMasterRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget)
    {
        for (FormattedBufferedStream &out : unit->getMasterMessageTargets()) {
            if (out.id() == answerTarget) {
                return out;
            }
        }
        throw 0;
    }

    FormattedBufferedStream &SyncableUnitBase::getSlaveMessageTarget() const
    {
        assert(mSynced);
        return mTopLevel->getSlaveMessageTarget();
    }

    void SyncableUnitBase::clearSlaveId(SerializeManager *mgr)
    {
        if (mSlaveId != 0) {
            mgr->removeSlaveMapping(this);
            mSlaveId = 0;
        }
    }

    SerializableUnitPtr SyncableUnitBase::customUnitPtr()
    {
        return { this, mType };
    }

    SerializableUnitConstPtr SyncableUnitBase::customUnitPtr() const
    {
        return { this, mType };
    }

    UnitId SyncableUnitBase::slaveId() const
    {
        return mSlaveId;
    }

    UnitId SyncableUnitBase::masterId() const
    {
        return mMasterId;
    }

    ParticipantId SyncableUnitBase::participantId() const
    {
        return mTopLevel ? mTopLevel->participantId() : SerializeManager::sLocalMasterParticipantId;
    }

    void SyncableUnitBase::setSlaveId(UnitId id, SerializeManager *mgr)
    {
        if (mTopLevel->getSlaveManager() != mgr) {
            assert(!mTopLevel->getSlaveManager());
            assert(mSlaveId == 0);
        }
        if (mSlaveId != id) {
            if (mSlaveId != 0) {
                mgr->removeSlaveMapping(this);
            }
            if (mTopLevel->getSlaveManager() == mgr)
                mSlaveId = id;
            mgr->addSlaveMapping(id, this);
        }
    }

    bool SyncableUnitBase::isMaster() const
    {
        return !mSynced || mSlaveId == 0;
    }

    UnitId SyncableUnitBase::moveMasterId(UnitId newId)
    {
        UnitId oldId = mMasterId;
        SerializeManager::deleteMasterId(mMasterId, this);
        mMasterId = SerializeManager::generateMasterId(newId, this);
        return oldId;
    }

    const SerializeTable *SyncableUnitBase::serializeType() const
    {
        return mType;
    }

    void SyncableUnitBase::writeFunctionAction(uint16_t index, const void *args, const std::set<ParticipantId> &targets, ParticipantId answerTarget, MessageId answerId)
    {
        std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> outStreams = getMasterMessageTargets(targets);
        for (FormattedBufferedStream &out : outStreams) {
            out.beginMessageWrite();
            SyncManager::writeActionHeader(out, this, MessageType::FUNCTION_ACTION, out.id() == answerTarget ? answerId : 0);
        }
        mType->writeFunctionArguments(outStreams, index, CALL, args);
        for (FormattedBufferedStream &out : outStreams) {
            out.endMessageWrite();
        }
    }

    void SyncableUnitBase::writeFunctionResult(uint16_t index, const void *result, FormattedBufferedStream &target, MessageId answerId)
    {
        assert(answerId != 0);
        target.beginMessageWrite();
        SyncManager::writeActionHeader(target, this, MessageType::FUNCTION_ACTION, answerId);
        mType->writeFunctionResult(target, index, result);
        target.endMessageWrite();
    }

    void SyncableUnitBase::writeFunctionRequest(uint16_t index, FunctionType type, const void *args, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver)
    {
        FormattedBufferedStream &out = getSlaveMessageTarget();
        out.beginMessageWrite();
        SyncManager::writeHeader(out, this, MessageType::FUNCTION_REQUEST);
        mType->writeFunctionArguments({ out }, index, type, args);
        out.endMessageWrite(requester, requesterTransactionId, std::move(receiver));
    }

    void SyncableUnitBase::writeFunctionError(uint16_t index, MessageResult error, FormattedBufferedStream &target, MessageId answerId)
    {
        assert(answerId != 0);
        target.beginMessageWrite();
        SyncManager::writeActionHeader(target, this, MessageType::FUNCTION_ERROR, answerId);
        mType->writeFunctionError(target, index, error);
        target.endMessageWrite();
    }

    StreamResult SyncableUnitBase::readFunctionAction(FormattedBufferedStream &in, PendingRequest &request)
    {
        return mType->readFunctionAction(this, in, request);
    }

    StreamResult SyncableUnitBase::readFunctionRequest(FormattedBufferedStream &in, MessageId id)
    {
        return mType->readFunctionRequest(this, in, id);
    }

    StreamResult SyncableUnitBase::readFunctionError(FormattedBufferedStream &in, PendingRequest &request)
    {
        return mType->readFunctionError(this, in, request);
    }

    std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> getMasterActionMessageTargets(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId,
        const std::set<ParticipantId> &targets)
    {
        std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> result = unit->getMasterMessageTargets();
        if (targets.empty()) {
            for (FormattedBufferedStream &out : result) {
                out.beginMessageWrite();
                SyncManager::writeActionHeader(out, unit, MessageType::ACTION, out.id() == answerTarget ? answerId : 0);
            }
        } else {
            auto it1 = result.begin();
            auto it2 = targets.begin();
            while (it1 != result.end() && it2 != targets.end()) {
                FormattedBufferedStream &out = *it1;
                while (*it2 < out.id()) {
                    throw 0; //LOG_WARNING("Specific Target not in MessageTargetList!");
                    ++it2;
                }
                if (*it2 == out.id()) {
                    out.beginMessageWrite();
                    SyncManager::writeActionHeader(out, unit, MessageType::ACTION, out.id() == answerTarget ? answerId : 0);
                    ++it2;
                    ++it1;
                } else {
                    it1 = result.erase(it1);
                }
            }
            result.erase(it1, result.end());
        }

        return result;
    }

    FormattedBufferedStream &getMasterRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId)
    {
        FormattedBufferedStream &out = getMasterRequestResponseTarget(unit, answerTarget);
        beginRequestResponseMessage(unit, out, answerId);
        return out;
    }

    FormattedBufferedStream &getSlaveRequestMessageTarget(const SyncableUnitBase *unit)
    {
        FormattedBufferedStream &out = unit->getSlaveMessageTarget();
        out.beginMessageWrite();
        SyncManager::writeHeader(out, unit, MessageType::REQUEST);
        return out;
    }

    void beginRequestResponseMessage(const SyncableUnitBase *unit, FormattedBufferedStream &stream, MessageId id)
    {
        stream.beginMessageWrite();
        SyncManager::writeActionHeader(stream, unit, MessageType::ACTION, id);
    }

    void SyncableUnitBase::writeAction(OffsetPtr offset, void *data, ParticipantId answerTarget, MessageId answerId, const std::set<ParticipantId> &targets) const
    {
        uint16_t index = mType->getIndex(offset);
        std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> streams = getMasterActionMessageTargets(this, answerTarget, answerId, targets);
        mType->writeAction(static_cast<const SerializableUnitBase*>(this), index, streams, data);
        for (FormattedBufferedStream &stream : streams)
            stream.endMessageWrite();
    }

    void SyncableUnitBase::writeRequest(OffsetPtr offset, void *data, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver) const
    {
        uint16_t index = mType->getIndex(offset);
        FormattedBufferedStream &stream = getSlaveRequestMessageTarget(this);
        mType->writeRequest(static_cast<const SerializableUnitBase *>(this), index, stream, data);
        stream.endMessageWrite(requester, requesterTransactionId, std::move(receiver));
    }

    void SyncableUnitBase::writeRequestResponse(OffsetPtr offset, void *data, ParticipantId answerTarget, MessageId answerId) const
    {
        if (answerTarget != 0) {
            uint16_t index = mType->getIndex(offset);
            FormattedBufferedStream &stream = Serialize::getMasterRequestResponseTarget(this, answerTarget, answerId);
            mType->writeAction(this, index, { stream }, data);
            stream.endMessageWrite();
        }
    }

    StreamResult tag_invoke(apply_map_t, SyncableUnitBase &unit, FormattedSerializeStream &in, bool success = true, const CallerHierarchyBasePtr &hierarchy = {})
    {
        return unit.mType->applyMap(&unit, in, success, hierarchy);
    }

}
}