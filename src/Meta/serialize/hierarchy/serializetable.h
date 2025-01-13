#pragma once

#include "serializetable_forward.h"

#include "Generic/callerhierarchy.h"

#include "Generic/closure.h"

#include "../primitivetypes.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    T unit_cast(void* unit) {
        if constexpr (std::derived_from<std::remove_pointer_t<T>, SerializableUnitBase>) {
            return static_cast<T>(static_cast<SerializableUnitBase *>(unit));
        } else {
            return static_cast<T>(unit);
        }
    }

    template <typename T>
    T unit_cast(const void *unit)
    {
        if constexpr (std::derived_from<std::remove_pointer_t<T>, SerializableUnitBase>) {
            return static_cast<T>(static_cast<const SerializableUnitBase *>(unit));
        } else {
            return static_cast<T>(unit);
        }
    }

    struct SerializeTableCallbacks {
        template <typename T>
        constexpr SerializeTableCallbacks(type_holder_t<T>)
            : onActivate([](void *unit, bool active, bool existenceChanged) {
                if constexpr (has_function_onActivate_v<T, bool, bool>)
                    unit_cast<T *>(unit)->onActivate(active, existenceChanged);
                else if constexpr (has_function_onActivate_v<T, bool>)
                    unit_cast<T *>(unit)->onActivate(active);
                else if constexpr (has_function_onActivate_v<T>)
                    unit_cast<T *>(unit)->onActivate();
            })
        {
        }

        void (*onActivate)(void *, bool, bool);
    };

    struct META_EXPORT SerializeTable {
        const char *mTypeName;
        SerializeTableCallbacks mCallbacks;
        const SerializeTable &(*mBaseType)();
        StreamResult (*mReadState)(const SerializeTable *, void *, FormattedSerializeStream &, StateTransmissionFlags, CallerHierarchyBasePtr);
        const Serializer *mFields;
        const SyncFunction *mFunctions;
        bool mIsTopLevelUnit;

        void writeState(const void *unit, FormattedSerializeStream &out, CallerHierarchyBasePtr hierarchy = {}) const;
        StreamResult readState(void *unit, FormattedSerializeStream &in, StateTransmissionFlags flags = 0, CallerHierarchyBasePtr hierarchy = {}) const;

        StreamResult readAction(void *unit, FormattedBufferedStream &in, PendingRequest &request) const;
        StreamResult readRequest(void *unit, FormattedBufferedStream &in, MessageId id) const;

        StreamResult applyMap(void *unit, FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy) const;
        void setSynced(SerializableUnitBase *unit, bool b, const CallerHierarchyBasePtr &hierarchy = {}) const;
        void setActive(void *unit, bool active, bool existenceChanged) const;
        void setActive(SerializableUnitBase *unit, bool active, bool existenceChanged) const;
        void setParent(SerializableUnitBase *unit) const;

        void writeAction(const void *unit, uint16_t index, const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, void *data) const;
        void writeRequest(const void *unit, uint16_t index, FormattedBufferedStream &out, void *data) const;

        StreamResult visitStream(FormattedSerializeStream &in, const StreamVisitor &visitor) const;

        uint16_t getIndex(OffsetPtr offset) const;
        const Serializer &get(uint16_t index) const;

        const SyncFunction &getFunction(uint16_t index) const;

        void writeFunctionArguments(const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, uint16_t index, FunctionType type, const void *args) const;
        void writeFunctionResult(FormattedBufferedStream &out, uint16_t index, const void *args) const;
        void writeFunctionError(FormattedBufferedStream &out, uint16_t index, MessageResult error) const;
        StreamResult readFunctionAction(SyncableUnitBase *unit, FormattedBufferedStream &in, PendingRequest &request) const;
        StreamResult readFunctionRequest(SyncableUnitBase *unit, FormattedBufferedStream &in, MessageId id) const;
        StreamResult readFunctionError(SyncableUnitBase *unit, FormattedBufferedStream &in, PendingRequest &request) const;
    };

}
}
