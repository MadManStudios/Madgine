#pragma once

#include "Generic/offsetptr.h"

#include "../streams/pendingrequest.h"
#include "serializableunit.h"
#include "serializetable_forward.h"
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
        void writeState(FormattedSerializeStream &out, const char *name = nullptr) const;
        StreamResult readState(FormattedSerializeStream &in, const char *name = nullptr);

        void setActive(bool active, bool existenceChanged, ContextPtr context);

        static StreamResult visitStream(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth);

        StreamResult readAction(FormattedSerializeStream &in, PendingRequest &request, ContextPtr context = {});
        StreamResult readRequest(FormattedMessageStream &in, MessageId id, ContextPtr context = {});

        StreamResult readFunctionAction(FormattedSerializeStream &in, PendingRequest &request, ContextPtr context = {});
        StreamResult readFunctionRequest(FormattedMessageStream &in, MessageId id, ContextPtr context = {});
        StreamResult readFunctionError(FormattedSerializeStream &in, PendingRequest &request, ContextPtr context = {});

        UnitId slaveId() const;
        UnitId masterId() const;
        ParticipantId participantId() const;

        bool isMaster() const;

        friend META_EXPORT WriteMessage getSlaveRequestMessageTarget(const SyncableUnitBase *unit, ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver);
        friend META_EXPORT std::vector<WriteMessage> getMasterActionMessageTargets(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId,
            const std::set<ParticipantId> &targets);
        friend META_EXPORT WriteMessage getMasterRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget, MessageId answerId);
        friend META_EXPORT FormattedMessageStream &getMasterFunctionRequestResponseTarget(const SyncableUnitBase *unit, ParticipantId answerTarget);

        friend META_EXPORT WriteMessage beginRequestResponseMessage(const SyncableUnitBase *unit, FormattedMessageStream &stream, MessageId id);

    protected:
        void writeId(FormattedSerializeStream &out, const char *name = nullptr) const;
        StreamResult readId(FormattedSerializeStream &in, const char *name = nullptr);
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
        void writeRequest(OffsetPtr offset, void *data, ParticipantId requester = 0, MessageId requesterTransactionId = 0, GenericMessageReceiver receiver = {}, ContextPtr context = {}) const;
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

        friend META_EXPORT StreamResult tag_invoke(apply_map_t, SyncableUnitBase &unit, FormattedSerializeStream &in, bool success, ContextPtr context);
        template <typename... Configs, typename Context>
        friend void tag_invoke(set_active_t<Configs...>, SyncableUnitBase &unit, bool active, bool existenceChanged, Context &&context)
        {
            unit.setActive(active, existenceChanged, context);
        }
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
            using R = typename Callable<f>::traits::return_type;

            if constexpr (std::same_as<R, void>) {
                Execution::Promise<MessageResult> promise;
                Execution::Future<MessageResult> future = promise.getFuture();
                typename context_args<decltype(f)>::as_tuple argTuple { std::forward<Args>(args)... };
                if (this->isMaster()) {
                    this->writeFunctionAction(functionIndex<f>, &argTuple);
                    TupleUnpacker::invokeExpand(f, static_cast<T *>(this), argTuple);
                    promise.set_value();
                } else {
                    this->writeFunctionRequest(functionIndex<f>, CALL, &argTuple, 0, 0, std::move(promise));
                }
                return future;
            } else {
                Execution::Promise<MessageResult, R> promise;
                Execution::Future<MessageResult, R> future = promise.getFuture();
                typename context_args<decltype(f)>::as_tuple argTuple { std::forward<Args>(args)... };
                if (this->isMaster()) {
                    this->writeFunctionAction(functionIndex<f>, &argTuple);
                    promise.set_value(TupleUnpacker::invokeExpand(f, static_cast<T *>(this), argTuple));
                } else {
                    this->writeFunctionRequest(functionIndex<f>, CALL, &argTuple, 0, 0, std::move(promise));
                }
                return future;
            }
        }

        template <auto f, typename... Args>
        void notify_some(const std::set<ParticipantId> &targets, Args &&...args)
        {
            assert(this->isMaster());
            if (!targets.empty()) {
                typename context_args<decltype(f)>::as_tuple argTuple { std::forward<Args>(args)... };
                this->writeFunctionAction(functionIndex<f>, &argTuple, targets);
            }
        }

        template <auto f, typename... Args>
            requires std::constructible_from<typename context_args<decltype(f)>::as_tuple, Args...>
        void notify(Args &&...args)
        {
            assert(this->isMaster());
            typename context_args<decltype(f)>::as_tuple argTuple { std::forward<Args>(args)... };
            this->writeFunctionAction(functionIndex<f>, &argTuple);
        }

        template <auto f, typename... Args>
        auto query(Args &&...args)
            requires std::constructible_from<typename context_args<decltype(f)>::as_tuple, Args...>
        {
            using R = typename Callable<f>::traits::return_type;
            using Tuple = typename context_args<decltype(f)>::as_tuple;
            Tuple argsTuple { std::forward<Args>(args)... };
            Execution::Promise<MessageResult, R> promise;
            Execution::Future<MessageResult, R> future = promise.getFuture();
            if (this->isMaster()) {

                StreamResult result = context_invoke([&](auto &&...contextArgs) -> StreamResult {
                    TupleUnpacker::invokeFromTuple([&](auto &&...args) {
                        if constexpr (std::same_as<R, void>) {
                            std::invoke(f, static_cast<T *>(this), std::forward<decltype(args)>(args)..., std::forward<decltype(contextArgs)>(contextArgs)...);
                            promise.set_value();
                        } else {
                            promise.set_value(std::invoke(f, static_cast<T *>(this), std::forward<decltype(args)>(args)..., std::forward<decltype(contextArgs)>(contextArgs)...));
                        }
                    },
                        std::move(argsTuple));
                    return {};
                },
                    context_contextual<decltype(f)> {}, SyncFunctionContext { sLocalMasterParticipantId });

                assert(!result.mError);

            } else {
                this->writeFunctionRequest(functionIndex<f>, QUERY, &argsTuple, 0, 0, std::move(promise));
            }
            return future;
        }

        template <auto f, typename... Args>
        void command(Args &&...args)
            requires std::constructible_from<typename context_args<decltype(f)>::as_tuple, Args...>
        {
            using Tuple = typename context_args<decltype(f)>::as_tuple;
            Tuple argTuple { std::forward<Args>(args)... };
            if (this->isMaster()) {
                StreamResult result = context_invoke([&](auto &&...contextArgs) -> StreamResult {
                    TupleUnpacker::invokeFromTuple([&](auto &&...args) {
                        std::invoke(f, static_cast<T *>(this), std::forward<decltype(args)>(args)..., std::forward<decltype(contextArgs)>(contextArgs)...);
                    },
                        std::move(argTuple));
                    return {};
                },
                    context_contextual<decltype(f)> {}, SyncFunctionContext { sLocalMasterParticipantId });

                assert(!result.mError);
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
