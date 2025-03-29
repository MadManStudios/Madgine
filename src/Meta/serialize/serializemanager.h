#pragma once



namespace Engine {
namespace Serialize {

    struct META_EXPORT SerializeManager {

        SerializeManager(const std::string &name);
        SerializeManager(const SerializeManager &) = delete;
        SerializeManager(SerializeManager &&) noexcept;
        ~SerializeManager();

        const SyncableUnitMap &mastersMap() const;

        void addSlaveMapping(UnitId id, SyncableUnitBase *item);
        void removeSlaveMapping(SyncableUnitBase *item);

        static UnitId generateMasterId(UnitId id, SyncableUnitBase *unit);
        static UnitId updateMasterId(UnitId id, SyncableUnitBase *unit);
        static void deleteMasterId(UnitId id, SyncableUnitBase *unit);

        bool isMaster(SerializeStreamData &stream) const;
        bool isMaster() const;

        static UnitId convertPtr(SerializeStream &stream, const SyncableUnitBase *unit);
        static StreamResult convertPtr(SerializeStream &stream, UnitId unit, SyncableUnitBase *&out);

        const std::string &name() const;

		SerializeStreamData *getSlaveStreamData();        

        SerializeStream wrapStream(Stream stream, bool isSlave = false);

    protected:
        void setSlaveStreamData(SerializeStreamData *data);

        std::unique_ptr<SerializeStreamData> createStreamData(ParticipantId id = createStreamId());
        static ParticipantId createStreamId();

		static SyncableUnitBase *getByMasterId(UnitId unit);

        SyncableUnitMap mSlaveMappings;   

    private:
		SerializeStreamData *mSlaveStreamData = nullptr;

        std::string mName;
    };
}
}
