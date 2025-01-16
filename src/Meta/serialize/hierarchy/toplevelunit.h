#pragma once

#include "syncableunit.h"

namespace Engine {
namespace Serialize {
    struct META_EXPORT TopLevelUnitBase : SyncableUnitBase {
        TopLevelUnitBase(UnitId masterId = 0);
        TopLevelUnitBase(const TopLevelUnitBase &other);
        TopLevelUnitBase(TopLevelUnitBase &&other) noexcept;
        ~TopLevelUnitBase();

        void sync();
        void unsync();

        FormattedMessageStream &getSlaveMessageTarget() const;

        const std::vector<SyncManager *> &getManagers() const;
        SyncManager *getSlaveManager() const;

        bool addManager(SyncManager *mgr);
        void removeManager(SyncManager *mgr);

        bool updateManagerType(SyncManager *mgr, bool isMaster);

        ParticipantId participantId() const;

        void setStaticSlaveId(UnitId slaveId);
        void receiveStateImpl(Execution::VirtualReceiverBase<bool> &receiver, SyncManager *mgr);
        ASYNC_STUB(receiveState, receiveStateImpl, Execution::make_simple_virtual_sender<bool>);
        void stateReadDone();

        std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> getMasterMessageTargets() const;

    private:
        std::vector<SyncManager *> mManagers;
        SyncManager *mSlaveManager = nullptr;
        UnitId mStaticSlaveId = 0;

        friend struct SyncManager;

        Execution::VirtualReceiverBase<bool> *mReceivingMasterState = nullptr;
    };

    template <typename T>
    using TopLevelUnit = SyncableUnit<T, TopLevelUnitBase>;
} // namespace Serialize
} // namespace Core
