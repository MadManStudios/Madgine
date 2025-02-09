#include "../metalib.h"

#include "serializemanager.h"

#include "hierarchy/serializableids.h"

#include "hierarchy/syncableunit.h"

#include "streams/serializestream.h"

#include "streams/serializestreamdata.h"

namespace Engine {
namespace Serialize {

    static std::mutex sMasterMappingMutex;
    static SyncableUnitMap sMasterMappings;
    static UnitId sNextUnitId = RESERVED_ID_COUNT;
    static std::atomic<ParticipantId> sRunningStreamId = sLocalMasterParticipantId;

    SerializeManager::SerializeManager(const std::string &name)
        : mName(name)
    {
    }

    SerializeManager::SerializeManager(SerializeManager &&other) noexcept
        : mSlaveMappings(std::move(other.mSlaveMappings))
        , mSlaveStreamData(std::exchange(other.mSlaveStreamData, nullptr))
        , mName(std::move(other.mName))
    {
        other.mSlaveMappings.clear();
    }

    SerializeManager::~SerializeManager() { }

    const SyncableUnitMap &SerializeManager::mastersMap() const
    {
        return sMasterMappings;
    }

    void SerializeManager::addSlaveMapping(UnitId id, SyncableUnitBase *item)
    {
        assert(mSlaveMappings.find(id) == mSlaveMappings.end());
        mSlaveMappings[id] = item;
        // std::cout << "Slave: " << item->slaveId() << " -> add " <<
        // typeid(*item).name() << std::endl;
    }

    void SerializeManager::removeSlaveMapping(SyncableUnitBase *item)
    {
        size_t result = mSlaveMappings.erase(item->slaveId());
        assert(result == 1);
    }

    UnitId SerializeManager::generateMasterId(UnitId id,
        SyncableUnitBase *unit)
    {
        if (id == 0) {
            std::lock_guard guard(sMasterMappingMutex);
            id = ++sNextUnitId;
            sMasterMappings[id] = unit;
        } else {
            assert(id >= BEGIN_USER_ID_SPACE);
            if (id >= RESERVED_ID_COUNT) {
                bool b = sMasterMappings.try_emplace(id, unit).second;
                assert(b);
            }
        }
        return id;
    }

    UnitId SerializeManager::updateMasterId(UnitId id, SyncableUnitBase *unit)
    {
        if (id >= RESERVED_ID_COUNT) {
            std::lock_guard guard(sMasterMappingMutex);
            auto it = sMasterMappings.find(id);
            assert(it->second != unit);
            it->second = unit;
        } else {
            assert(id >= BEGIN_USER_ID_SPACE);
        }
        return id;
    }

    void SerializeManager::deleteMasterId(UnitId id, SyncableUnitBase *unit)
    {
        if (id >= RESERVED_ID_COUNT) {
            std::lock_guard guard(sMasterMappingMutex);
            auto it = sMasterMappings.find(id);
            assert(it->second == unit);
            sMasterMappings.erase(it);
        } else {
            assert(id >= BEGIN_USER_ID_SPACE);
        }
    }

    bool SerializeManager::isMaster(SerializeStreamData &stream) const
    {
        return mSlaveStreamData != &stream;
    }

    bool SerializeManager::isMaster() const
    {
        return !mSlaveStreamData;
    }

    UnitId SerializeManager::convertPtr(SerializeStream &out,
        const SyncableUnitBase *unit)
    {
        return unit == nullptr
            ? NULL_UNIT_ID
            : out.isMaster(AccessMode::WRITE) ? unit->masterId()
                                              : unit->slaveId();
    }

    StreamResult SerializeManager::convertPtr(SerializeStream &in,
        UnitId unit, SyncableUnitBase *&out)
    {
        if (unit == NULL_UNIT_ID) {
            out = nullptr;
            return {};
        }
        SyncableUnitBase *ptr = nullptr;

        if (in.isMaster(AccessMode::READ)) {
            ptr = getByMasterId(unit);
        } else {
            auto it = in.manager()->mSlaveMappings.find(unit);
            if (it != in.manager()->mSlaveMappings.end())
                ptr = it->second;
        }
        if (!ptr) {
            return STREAM_INTEGRITY_ERROR(in, true) << "Unknown Unit-Id (" << unit << ") used!";
        }
        out = ptr;
        return {};
    }

    SerializeStreamData *SerializeManager::getSlaveStreamData()
    {
        return mSlaveStreamData;
    }

    SerializeStream SerializeManager::wrapStream(Stream stream, bool isSlave)
    {
        std::unique_ptr<SerializeStreamData> data = createStreamData();
        if (isSlave)
            setSlaveStreamData(data.get());
        return SerializeStream {
            stream.release(),
            std::move(data)
        };
    }

    void SerializeManager::setSlaveStreamData(SerializeStreamData *data)
    {
        if (mSlaveStreamData && data)
            std::terminate();
        mSlaveStreamData = data;
    }

    std::unique_ptr<SerializeStreamData> SerializeManager::createStreamData(ParticipantId id)
    {
        return std::make_unique<SerializeStreamData>(*this, id);
    }

    ParticipantId SerializeManager::createStreamId() { return ++sRunningStreamId; }

    SyncableUnitBase *SerializeManager::getByMasterId(UnitId unit)
    {
        std::lock_guard guard(sMasterMappingMutex);
        auto it = sMasterMappings.find(unit);
        if (it == sMasterMappings.end())
            return nullptr;
        return it->second;
    }

    const std::string &SerializeManager::name() const
    {
        return mName;
    }

    //const std::string &SerializeManager::name() const { return mName; }
} // namespace Serialize
} // namespace Engine
