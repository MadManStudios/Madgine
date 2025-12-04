#pragma once

#include "Generic/callerhierarchy.h"
#include "Generic/closure.h"

#include "../primitivetypes.h"
#include "serializetable_forward.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    T *unit_cast(void *unit)
    {
        if constexpr (std::derived_from<std::remove_pointer_t<T>, SerializableUnitBase>) {
            return static_cast<T *>(static_cast<SerializableUnitBase *>(unit));
        } else {
            return static_cast<T *>(unit);
        }
    }

    template <typename T>
    const T *unit_cast(const void *unit)
    {
        if constexpr (std::derived_from<std::remove_pointer_t<T>, SerializableUnitBase>) {
            return static_cast<const T *>(static_cast<const SerializableUnitBase *>(unit));
        } else {
            return static_cast<const T *>(unit);
        }
    }

    struct SerializeTableCallbacks {

        template <typename T>
        constexpr SerializeTableCallbacks(type_holder_t<T>)
            : onActivate([](void *unit, CallbackTiming timing, bool active, bool existenceChanged) {
                if constexpr (requires(T *unit) { TupleUnpacker::invoke(&T::onActivate, unit, timing, active, existenceChanged); }) {
                    TupleUnpacker::invoke(&T::onActivate, unit_cast<T>(unit), timing, active, existenceChanged);
                }
            })
        {
        }

        void (*onActivate)(void *, CallbackTiming, bool, bool);
    };

    struct META_EXPORT SerializeTable {
        const char *mTypeName;
        SerializeTableCallbacks mCallbacks;
        const SerializeTable &(*mBaseType)();
        StreamResult (*mReadState)(const SerializeTable *, void *, CallerHierarchyFormattedSerializeStream);
        const Serializer *mFields;
        const SyncFunction *mFunctions;
        bool mIsTopLevelUnit;

        void writeState(const void *unit, CallerHierarchyFormattedSerializeStream out) const;
        StreamResult readState(void *unit, CallerHierarchyFormattedSerializeStream in) const;

        StreamResult readAction(void *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const;
        StreamResult readRequest(void *unit, FormattedMessageStream &in, MessageId id) const;

        StreamResult applyMap(void *unit, CallerHierarchyFormattedSerializeStream in, bool success) const;
        void setSynced(SerializableUnitBase *unit, bool b, const CallerHierarchyBasePtr &hierarchy = {}) const;
        void setActive(void *unit, bool active, bool existenceChanged) const;
        void setActive(SerializableUnitBase *unit, bool active, bool existenceChanged) const;
        void setParent(SerializableUnitBase *unit) const;

        void writeAction(const void *unit, uint16_t index, const std::vector<WriteMessage> &outStreams, void *data) const;
        void writeRequest(const void *unit, uint16_t index, CallerHierarchyFormattedSerializeStream out, void *data) const;

        StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const StreamVisitor &visitor, size_t depth) const;

        uint16_t getIndex(OffsetPtr offset) const;
        const Serializer &get(uint16_t index) const;

        const SyncFunction &getFunction(uint16_t index) const;

        void writeFunctionArguments(const std::vector<WriteMessage> &outStreams, uint16_t index, FunctionType type, const void *args) const;
        void writeFunctionResult(CallerHierarchyFormattedSerializeStream out, uint16_t index, const void *args) const;
        void writeFunctionError(CallerHierarchyFormattedSerializeStream out, uint16_t index, MessageResult error) const;
        StreamResult readFunctionAction(SyncableUnitBase *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const;
        StreamResult readFunctionRequest(SyncableUnitBase *unit, FormattedMessageStream &in, MessageId id) const;
        StreamResult readFunctionError(SyncableUnitBase *unit, CallerHierarchyFormattedSerializeStream in, PendingRequest &request) const;
    };

}
}
