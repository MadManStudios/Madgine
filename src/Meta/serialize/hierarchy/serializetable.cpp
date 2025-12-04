#include "../../metalib.h"

#include "serializetable.h"

#include "Generic/offsetptr.h"

#include "../operations.h"
#include "../streams/comparestreamid.h"
#include "../streams/formattedmessagestream.h"
#include "../streams/writemessage.h"
#include "serializableunit.h"
#include "serializer.h"
#include "syncableunit.h"
#include "syncfunction.h"

namespace Engine {
namespace Serialize {

    void writeFunctionAction(SyncableUnitBase *unit, uint16_t index, const void *args, const std::set<ParticipantId> &targets, ParticipantId answerTarget, MessageId answerId)
    {
        unit->writeFunctionAction(index, args, targets, answerTarget, answerId);
    }

    void writeFunctionResult(SyncableUnitBase *unit, uint16_t index, const void *result, FormattedMessageStream &target, MessageId answerId)
    {
        unit->writeFunctionResult(index, result, target, answerId);
    }

    void writeFunctionRequest(SyncableUnitBase *unit, uint16_t index, FunctionType type, const void *args, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver)
    {
        unit->writeFunctionRequest(index, type, args, requester, requesterTransactionId, std::move(receiver));
    }

    void writeFunctionError(SyncableUnitBase *unit, uint16_t index, MessageResult error, FormattedMessageStream &target, MessageId answerId)
    {
        unit->writeFunctionError(index, error, target, answerId);
    }

    StreamResult META_EXPORT readState(const SerializeTable *table, void *unit, CallerHierarchyFormattedSerializeStream in)
    {
        if (in.mStream.supportsNameLookup()) {
            std::string name;
            STREAM_PROPAGATE_ERROR(in.mStream.lookupFieldName(name));
            while (!name.empty()) {
                bool found = false;
                const SerializeTable *tableAcc = table;
                while (tableAcc && !found) {
                    for (const Serializer *it = tableAcc->mFields; it->mFieldName; ++it) {
                        if (name == it->mFieldName) {
                            STREAM_PROPAGATE_ERROR(it->mReadState(unit, in, it->mFieldName));
                            found = true;
                            break;
                        }
                    }
                    tableAcc = tableAcc->mBaseType ? &tableAcc->mBaseType() : nullptr;
                }
                if (!found)
                    return STREAM_PARSE_ERROR(in) << "Could not find field '" << name << "' in type '" << table->mTypeName << "'";
                STREAM_PROPAGATE_ERROR(in.mStream.lookupFieldName(name));
            }
        } else {
            const SerializeTable *tableAcc = table;
            while (tableAcc) {
                for (const Serializer *it = tableAcc->mFields; it->mFieldName; ++it) {
                    STREAM_PROPAGATE_ERROR(it->mReadState(unit, in, it->mFieldName));
                }
                tableAcc = tableAcc->mBaseType ? &tableAcc->mBaseType() : nullptr;
            }
        }
        return {};
    }

    void SerializeTable::writeState(const void *unit, CallerHierarchyFormattedSerializeStream out) const
    {
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                it->mWriteState(unit, out, it->mFieldName);
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }
    }

    StreamResult SerializeTable::readState(void *unit, CallerHierarchyFormattedSerializeStream in) const
    {
        return mReadState(this, unit, in);
    }

    StreamResult SerializeTable::readAction(void *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const
    {
        uint16_t index;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, index, "index"));
        return get(index).mReadAction(unit, in, request);
    }

    StreamResult SerializeTable::readRequest(void *unit, FormattedMessageStream &inout, MessageId id) const
    {
        uint16_t index;
        STREAM_PROPAGATE_ERROR(read(inout, index, "index"));
        return get(index).mReadRequest(unit, inout, id);
    }

    StreamResult SerializeTable::applyMap(void *unit, CallerHierarchyFormattedSerializeStream in, bool success) const
    {
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                STREAM_PROPAGATE_ERROR(it->mApplySerializableMap(it, unit, in, success));
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }
        return {};
    }

    void SerializeTable::setSynced(SerializableUnitBase *unit, bool b, const CallerHierarchyBasePtr &hierarchy) const
    {
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                it->mSetDataSynced(it, unit, b, hierarchy);
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }
    }

    void SerializeTable::setActive(void *unit, bool active, bool existenceChanged) const
    {
        if (mCallbacks.onActivate)
            mCallbacks.onActivate(unit, CallbackTiming::BEFORE, active, existenceChanged);

        // TODO: Start with base
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                it->mSetActive(it, unit, active, existenceChanged);
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }
        if (mCallbacks.onActivate)
            mCallbacks.onActivate(unit, CallbackTiming::AFTER, active, existenceChanged);
    }

    void SerializeTable::setActive(SerializableUnitBase *unit, bool active, bool existenceChanged) const
    {
        if (active)
            assert(unit->mActiveIndex == 0);

        if (mCallbacks.onActivate)
            mCallbacks.onActivate(unit, CallbackTiming::BEFORE, active, existenceChanged);

        // TODO: Start with base
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                if (active)
                    ++unit->mActiveIndex;
                else {
                    assert(unit->mActiveIndex > 0);
                    --unit->mActiveIndex;
                }
                it->mSetActive(it, unit, active, existenceChanged);
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }

        if (!active)
            assert(unit->mActiveIndex == 0);

        if (mCallbacks.onActivate)
            mCallbacks.onActivate(unit, CallbackTiming::AFTER, active, existenceChanged);
    }

    void SerializeTable::setParent(SerializableUnitBase *unit) const
    {
        const SerializeTable *table = this;
        while (table) {
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                it->mSetParent(it, unit);
            }
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }
    }

    void SerializeTable::writeAction(const void *unit, uint16_t index, const std::vector<WriteMessage> &outStreams, void *data) const
    {
        for (const WriteMessage &out : outStreams) {
            Serialize::write(out, index, "index");
        }
        get(index).mWriteAction(unit, outStreams, data);
    }

    void SerializeTable::writeRequest(const void *unit, uint16_t index, CallerHierarchyFormattedSerializeStream out, void *data) const
    {
        write(out, index, "index");
        get(index).mWriteRequest(unit, out, data);
    }

    StreamResult SerializeTable::visitStream(CallerHierarchyFormattedSerializeStream in, const StreamVisitor &visitor, size_t depth) const
    {
        if (in.mStream.supportsNameLookup()) {
            std::string name;
            STREAM_PROPAGATE_ERROR(in.mStream.lookupFieldName(name));
            while (!name.empty()) {
                bool found = false;
                const SerializeTable *tableAcc = this;
                while (tableAcc && !found) {
                    for (const Serializer *it = tableAcc->mFields; it->mFieldName; ++it) {
                        if (name == it->mFieldName) {
                            STREAM_PROPAGATE_ERROR(it->mVisitStream(in, it->mFieldName, visitor, depth + 1));
                            found = true;
                            break;
                        }
                    }
                    tableAcc = tableAcc->mBaseType ? &tableAcc->mBaseType() : nullptr;
                }
                if (!found)
                    return STREAM_PARSE_ERROR(in) << "Could not find field '" << name << "'";
                STREAM_PROPAGATE_ERROR(in.mStream.lookupFieldName(name));
            }
        } else {
            const SerializeTable *tableAcc = this;
            while (tableAcc) {
                for (const Serializer *it = tableAcc->mFields; it->mFieldName; ++it) {
                    STREAM_PROPAGATE_ERROR(it->mVisitStream(in, it->mFieldName, visitor, depth + 1));
                }
                tableAcc = tableAcc->mBaseType ? &tableAcc->mBaseType() : nullptr;
            }
        }
        return {};
    }

    uint16_t SerializeTable::getIndex(OffsetPtr offset) const
    {
        uint16_t index = 0;

        std::stack<const SerializeTable *> tables;
        const SerializeTable *table = this;
        while (table) {
            tables.push(table);
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }

        while (!tables.empty()) {
            table = tables.top();
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                if (it->mOffset == offset) {
                    return index;
                }
                ++index;
            }
            tables.pop();
        }

        assert(false);
        throw 0;
    }

    const Serializer &SerializeTable::get(uint16_t index) const
    {
        std::stack<const SerializeTable *> tables;
        const SerializeTable *table = this;
        while (table) {
            tables.push(table);
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }

        while (!tables.empty()) {
            table = tables.top();
            for (const Serializer *it = table->mFields; it->mFieldName; ++it) {
                if (index == 0) {
                    return *it;
                }
                --index;
            }
            tables.pop();
        }

        // Corrupt package
        std::terminate();
    }

    const SyncFunction &SerializeTable::getFunction(uint16_t index) const
    {
        std::stack<const SerializeTable *> tables;
        const SerializeTable *table = this;
        while (table) {
            tables.push(table);
            table = table->mBaseType ? &table->mBaseType() : nullptr;
        }

        while (!tables.empty()) {
            table = tables.top();
            for (const SyncFunction *it = table->mFunctions; it; ++it) {
                if (index == 0) {
                    return *it;
                }
                --index;
            }
            tables.pop();
        }

        // Corrupt package
        std::terminate();
    }

    void SerializeTable::writeFunctionArguments(const std::vector<WriteMessage> &outStreams, uint16_t index, FunctionType type, const void *args) const
    {
        for (const WriteMessage &out : outStreams) {
            Serialize::write(out, index, "index");
            Serialize::write(out, type, "functionType");
        }
        getFunction(index).mWriteFunctionArguments(outStreams, args);
    }

    void SerializeTable::writeFunctionResult(CallerHierarchyFormattedSerializeStream out, uint16_t index, const void *args) const
    {
        Serialize::write(out, index, "index");
        Serialize::write(out, QUERY, "functionType");
        getFunction(index).mWriteFunctionResult(out, args);
    }

    void SerializeTable::writeFunctionError(CallerHierarchyFormattedSerializeStream out, uint16_t index, MessageResult error) const
    {
        write(out, error, "error");
    }

    StreamResult SerializeTable::readFunctionAction(SyncableUnitBase *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const
    {
        uint16_t index;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, index, "index"));
        FunctionType type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "functionType"));
        return getFunction(index).mReadFunctionAction(unit, in, index, type, request);
    }

    StreamResult SerializeTable::readFunctionRequest(SyncableUnitBase *unit, FormattedMessageStream &in, MessageId id) const
    {
        uint16_t index;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, index, "index"));
        FunctionType type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "functionType"));
        return getFunction(index).mReadFunctionRequest(unit, in, index, type, id);
    }

    StreamResult SerializeTable::readFunctionError(SyncableUnitBase *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const
    {
        MessageResult error;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, error, "error"));
        request.mReceiver.set_error(error);
        return {};
    }

}
}