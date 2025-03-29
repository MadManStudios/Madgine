#pragma once

#include "../streams/pendingrequest.h"
#include "Generic/offsetptr.h"
#include "serializableunit.h"
#include "syncfunction.h"

namespace Engine {
namespace Serialize {

#define SYNCABLEUNIT_MEMBERS()            \
    SERIALIZABLEUNIT_MEMBERS()            \
    READONLY_PROPERTY(MasterId, masterId) \
    READONLY_PROPERTY(SlaveId, slaveId)

    template <auto f>
    constexpr uint16_t functionIndex = __serialize_impl__::SyncFunctionTable<typename Callable<f>::traits::class_type>::template getIndex<f>();

    struct META_EXPORT SyncableUnitBase : SerializableUnitBase {
    protected:
        SyncableUnitBase(UnitId masterId = 0);
        SyncableUnitBase(const SyncableUnitBase &other);
        SyncableUnitBase(SyncableUnitBase &&other) noexcept;
        ~SyncableUnitBase();

        SyncableUnitBase &operator=(const SyncableUnitBase &other);
        SyncableUnitBase &operator=(SyncableUnitBase &&other);

    public:
        void writeState(FormattedSerializeStream &out, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}) const;
        StreamResult readState(FormattedSerializeStream &in, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {});

        void setActive(bool active, bool existenceChanged);

        static StreamResult visitStream(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);

        StreamResult readAction(FormattedMessageStream &in, PendingRequest &request);
        StreamResult readRequest(FormattedMessageStream &in, MessageId id);

        StreamResult readFunctionAction(FormattedMessageStream &in, PendingRequest &request);
        StreamResult readFunctionRequest(FormattedMessageStream &in, MessageId id);
        StreamResult readFunctionError(FormattedMessageStream &in, PendingRequest &request);

        UnitId slaveId() const;
        UnitId masterId() const;
        ParticipantId participantId() const;

        bool isMaster() const;

        friend META_EXPORT WriteMessage getSlaveRequestMessageTarget(const SyncableUnitBase *unit, ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver);
        friend META_EXPORT std::vector<WriteMessage> getMasterActionMessageTargets(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId,
            const std::set<ParticipantId> &targets);
        friend META_EXPORT WriteMessage getMasterRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId);

        friend META_EXPORT WriteMessage beginRequestResponseMessage(const SyncableUnitBase *unit, FormattedMessageStream &stream, MessageId id);

    protected:
        void setSlaveId(UnitId id, SerializeManager *mgr);

        const SerializeTable *serializeType() const;

        UnitId moveMasterId(UnitId newId = 0);

        friend META_EXPORT void writeFunctionAction(SyncableUnitBase *unit, uint16_t index, const void *args, const std::set<ParticipantId> &targets, ParticipantId answerTarget, MessageId answerId);
        friend META_EXPORT void writeFunctionResult(SyncableUnitBase *unit, uint16_t index, const void *result, FormattedMessageStream &target, MessageId answerId);
        friend META_EXPORT void writeFunctionRequest(SyncableUnitBase *unit, uint16_t index, FunctionType type, const void *args, ParticipantId requester, MessageId requesterTransactionId, GenericMessageReceiver receiver);
        friend META_EXPORT void writeFunctionError(SyncableUnitBase *unit, uint16_t index, MessageResult error, FormattedMessageStream &target, MessageId answerId);

        void writeFunctionAction(uint16_t index, const void *args, const std::set<ParticipantId> &targets = {}, ParticipantId answerTarget = 0, MessageId answerId = 0);
        void writeFunctionResult(uint16_t index, const void *result, FormattedMessageStream &target, MessageId answerId);
        void writeFunctionRequest(uint16_t index, FunctionType type, const void *args, ParticipantId requester = 0, MessageId requesterTransactionId = 0, GenericMessageReceiver receiver = {});
        void writeFunctionError(uint16_t index, MessageResult error, FormattedMessageStream &target, MessageId answerId);

        void writeAction(OffsetPtr offset, void *data, ParticipantId answerTarget, MessageId answerId, const std::set<ParticipantId> &targets = {}) const;
        void writeRequest(OffsetPtr offset, void *data, ParticipantId requester = 0, MessageId requesterTransactionId = 0, GenericMessageReceiver receiver = {}) const;
        void writeRequestResponse(OffsetPtr offset, void *data, ParticipantId answerTarget, MessageId answerId) const;

    private:
        std::set<std::reference_wrapper<FormattedMessageStream>, CompareStreamId> getMasterMessageTargets(const std::set<ParticipantId> &targets = {}) const;
        FormattedMessageStream &getSlaveMessageTarget() const;

        void clearSlaveId(SerializeManager *mgr);

        friend struct SyncManager;
        template <typename T, typename Base>
        friend struct TableInitializer;
        friend struct SerializableUnitConstPtr;
        friend struct SerializableUnitPtr;
        friend struct SerializableDataPtr;

        META_EXPORT friend StreamResult tag_invoke(apply_map_t, SyncableUnitBase &unit, FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy);
        friend META_EXPORT StreamResult convertSyncablePtr(FormattedSerializeStream &in, UnitId id, SyncableUnitBase *&out, const SerializeTable *&type);

        DERIVE_FRIEND(customUnitPtr)
        SerializableUnitPtr customUnitPtr();
        SerializableUnitConstPtr customUnitPtr() const;

    private:
        UnitId mSlaveId = 0;
        UnitId mMasterId;

        const SerializeTable *mType = nullptr;
    };

    template <typename T, typename Base>
    struct TableInitialized;

    template <typename T, typename Base>
    struct TableInitializer {
        TableInitializer()
        {
            static_cast<TableInitialized<T, Base> *>(this)->mType = &serializeTable<T>();
        }
        TableInitializer(const TableInitializer &)
        {
            static_cast<TableInitialized<T, Base> *>(this)->mType = &serializeTable<T>();
        }
        TableInitializer(TableInitializer &&)
        {
            static_cast<TableInitialized<T, Base> *>(this)->mType = &serializeTable<T>();
        }
        TableInitializer &operator=(const TableInitializer &)
        {
            return *this;
        }
        TableInitializer &operator=(TableInitializer &&)
        {
            return *this;
        }
    };

    template <typename T, typename _Base = SyncableUnitBase>
    struct TableInitialized : _Base, private TableInitializer<T, _Base> {
        friend TableInitializer<T, _Base>;

        using _Base::_Base;
    };

    template <typename T, typename _Base>
    struct SyncableUnitEx : _Base {

        using _Base::_Base;

        template <typename OffsetPtr>
        friend struct Syncable;

    protected:
        template <auto f, typename... Args>
        auto call(Args &&...args)
        {
            using traits = typename Callable<f>::traits;
            using R = typename traits::return_type;

            if constexpr (std::same_as<R, void>) {
                return make_message_sender<>(
                    [this](auto &receiver, Args &&...args2) {
                        typename traits::decay_argument_types::as_tuple argTuple { std::forward<Args>(args2)... };
                        if (this->isMaster()) {
                            this->writeFunctionAction(functionIndex<f>, &argTuple);
                            TupleUnpacker::invokeExpand(f, static_cast<T *>(this), argTuple);
                            receiver.set_value();
                        } else {
                            this->writeFunctionRequest(functionIndex<f>, CALL, &argTuple, 0, 0, receiver);
                        }
                    },
                    std::forward<Args>(args)...);
            } else {
                return make_message_sender<R>(
                    [this](auto &receiver, Args &&...args2) {
                        typename traits::decay_argument_types::as_tuple argTuple { std::forward<Args>(args2)... };
                        if (this->isMaster()) {
                            this->writeFunctionAction(functionIndex<f>, &argTuple);
                            receiver.set_value(TupleUnpacker::invokeExpand(f, static_cast<T *>(this), argTuple));
                        } else {
                            this->writeFunctionRequest(functionIndex<f>, CALL, &argTuple, 0, 0, receiver);
                        }
                    },
                    std::forward<Args>(args)...);
            }
        }

        template <auto f, typename... Args>
        void notify_some(const std::set<ParticipantId> &targets, Args &&...args)
        {
            assert(this->isMaster());
            if (!targets.empty()) {
                using traits = typename Callable<f>::traits;
                typename traits::decay_argument_types::as_tuple argTuple { std::forward<Args>(args)... };
                this->writeFunctionAction(functionIndex<f>, &argTuple, targets);
            }
        }

        template <auto f, typename... Args>
            requires std::constructible_from<typename SyncFunctionTraits<typename Callable<f>::traits>::decay_argument_types::as_tuple, Args...>
        void notify(Args &&...args)
        {
            assert(this->isMaster());
            using traits = typename Callable<f>::traits;
            typename traits::decay_argument_types::as_tuple argTuple { std::forward<Args>(args)... };
            this->writeFunctionAction(functionIndex<f>, &argTuple);
        }

        template <auto f, typename... Args>
        auto query(Args &&...args)
            requires std::constructible_from<typename SyncFunctionTraits<typename Callable<f>::traits>::decay_argument_types::as_tuple, Args...>
        {
            using traits = SyncFunctionTraits<typename Callable<f>::traits>;
            using R = typename traits::return_type;
            using Tuple = traits::decay_argument_types::as_tuple;
            return make_message_sender<R>(
                [this](auto &receiver, Args &&...args2) {
                    Tuple args { std::forward<Args>(args2)... };
                    if (this->isMaster()) {                        
                        if constexpr (std::same_as<decltype(TupleUnpacker::invokeExpand(f, static_cast<T *>(this), traits::patchArgs(std::move(args), { 1 }))), void>) {
                            TupleUnpacker::invokeExpand(f, static_cast<T *>(this), traits::patchArgs(std::move(args), { sLocalMasterParticipantId })); // TODO: Constant
                            receiver.set_value();
                        } else {
                            receiver.set_value(TupleUnpacker::invokeExpand(f, static_cast<T *>(this), traits::patchArgs(std::move(args), { sLocalMasterParticipantId })));
                        }
                    } else {
                        this->writeFunctionRequest(functionIndex<f>, QUERY, &args, 0, 0, receiver);
                    }
                },
                std::forward<Args>(args)...);
        }

        template <auto f, typename... Args>
        void command(Args &&...args)
            requires std::constructible_from<typename SyncFunctionTraits<typename Callable<f>::traits>::decay_argument_types::as_tuple, Args...>
        {
            using traits = SyncFunctionTraits<typename Callable<f>::traits>;
            using Tuple = typename traits::decay_argument_types::as_tuple;
            Tuple argTuple { std::forward<Args>(args)... };
            if (this->isMaster()) {
                TupleUnpacker::invokeExpand(f, static_cast<T *>(this), traits::patchArgs(std::move(argTuple), { sLocalMasterParticipantId }));
            } else {                 
                this->writeFunctionRequest(functionIndex<f>, QUERY, &argTuple);
            }
        }

        using _Base::writeAction;
        template <typename Ty, typename... Args>
        void writeAction(Ty *field, ParticipantId answerTarget, MessageId answerId, Args &&...args) const
        {
            OffsetPtr offset { static_cast<const T *>(this), field };
            typename Ty::action_payload data { std::forward<Args>(args)... };
            _Base::writeAction(offset, &data, answerTarget, answerId, {});
        }

        using _Base::writeRequest;
        template <typename Ty, typename... Args>
        void writeRequest(Ty *field, ParticipantId requester, MessageId requesterTransactionId, Args &&...args) const
        {
            OffsetPtr offset { static_cast<const T *>(this), field };
            typename Ty::request_payload data { std::forward<Args>(args)... };
            _Base::writeRequest(offset, &data, requester, requesterTransactionId);
        }

        template <typename Ty, typename... Args>
        void writeRequest(Ty *field, GenericMessageReceiver receiver, Args &&...args) const
        {
            OffsetPtr offset { static_cast<const T *>(this), field };
            typename Ty::request_payload data { std::forward<Args>(args)... };
            _Base::writeRequest(offset, &data, 0, 0, std::move(receiver));
        }

        using _Base::writeRequestResponse;
        template <typename Ty, typename... Args>
        void writeRequestResponse(Ty *field, ParticipantId answerTarget, MessageId answerId, Args &&...args) const
        {
            OffsetPtr offset { static_cast<const T *>(this), field };
            typename Ty::action_payload data { std::forward<Args>(args)... };
            _Base::writeRequestResponse(offset, &data, answerTarget, answerId);
        }
    };

    template <typename T, typename _Base = SyncableUnitBase>
    using SyncableUnit = SyncableUnitEx<T, TableInitialized<T, _Base>>;

} // namespace Serialize
} // namespace Core
