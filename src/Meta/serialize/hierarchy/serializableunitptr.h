#pragma once

#include "Generic/callerhierarchy.h"
#include "serializetable_forward.h"

#include "../streams/streamresult.h"

#include "../primitivetypes.h"

namespace Engine {
struct ParameterTuple;
namespace Serialize {

    DERIVE_FUNCTION(customUnitPtr)

    struct META_EXPORT SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableDataConstPtr(const T *t, std::true_type)
            : SerializableDataConstPtr(t ? t->customUnitPtr() : SerializableDataConstPtr { static_cast<const void*>(nullptr), &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableDataConstPtr(const T *t, std::false_type)
            : SerializableDataConstPtr(t, &serializeTable<decayed_t<T>>())
        {
        }

    public:
        constexpr SerializableDataConstPtr() = default;

        template <typename T>
        SerializableDataConstPtr(const T *t)
            : SerializableDataConstPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableDataConstPtr(const SerializableDataConstPtr &other)
            : mUnit(other.mUnit)
            , mType(other.mType)
        {
        }

        SerializableDataConstPtr(const void *unit, const SerializeTable *type)
            : mUnit(unit)
            , mType(type)
        {
        }

        SerializableDataConstPtr(const SerializableUnitBase *unit, const SerializeTable *type)
            : SerializableDataConstPtr(static_cast<const void *>(unit), type)
        {
        }

        bool operator==(const SerializableDataConstPtr &other) const
        {
            return mUnit == other.mUnit && mType == other.mType;
        }

        auto operator<=>(const SerializableDataConstPtr &other) const = default;

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        void writeState(FormattedSerializeStream &out, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        const void *mUnit = nullptr;
        const SerializeTable *mType = nullptr;
    };

    struct META_EXPORT SerializableDataPtr : SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableDataPtr(T *t, std::true_type)
            : SerializableDataPtr(t ? SerializableDataPtr { t->customUnitPtr() } : SerializableDataPtr { static_cast<void*>(nullptr), &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableDataPtr(T *t, std::false_type)
            : SerializableDataConstPtr(t)
        {
        }

    public:
        constexpr SerializableDataPtr() = default;

        template <typename T>
            requires(!std::is_const_v<T>)
        SerializableDataPtr(T *t)
            : SerializableDataPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableDataPtr(const SerializableDataPtr &other) = default;

        SerializableDataPtr(SerializableUnitBase *unit, const SerializeTable *type)
            : SerializableDataConstPtr(unit, type)
        {
        }

        SerializableDataPtr(void *unit, const SerializeTable *type)
            : SerializableDataConstPtr(unit, type)
        {
        }

        SerializableDataPtr(const SerializableUnitPtr &other);

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        StreamResult readState(FormattedSerializeStream &in, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        StreamResult applyMap(FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy) const;

        void setActive(bool active, bool existenceChanged) const;

        static StreamResult visitStream(const SerializeTable *type, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);
        template <typename T>
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            return visitStream(&serializeTable<decayed_t<T>>(), in, name, visitor);
        }

        void *unit() const;
    };

    struct META_EXPORT SerializableUnitConstPtr : SerializableDataConstPtr {
    private:
        template <typename T>
        SerializableUnitConstPtr(const T *t, std::true_type)
            : SerializableUnitConstPtr(t ? t->customUnitPtr() : SerializableUnitConstPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableUnitConstPtr(const T *t, std::false_type)
            : SerializableUnitConstPtr(t, &serializeTable<decayed_t<T>>())
        {
        }

    public:
        constexpr SerializableUnitConstPtr() = default;

        template <std::derived_from<SerializableUnitBase> T>
            requires(!std::same_as<SerializableUnitBase, T>)
        SerializableUnitConstPtr(const T *t)
            : SerializableUnitConstPtr(t, has_function_customUnitPtr<T> {})
        {
        }

        constexpr SerializableUnitConstPtr(const SerializableUnitConstPtr &other) = default;

        SerializableUnitConstPtr(const SerializableUnitBase *unit, const SerializeTable *type);

        bool operator==(const SerializableUnitConstPtr &other) const
        {
            return mUnit == other.mUnit && mType == other.mType;
        }

        /* bool operator<(const SerializableUnitConstPtr &other) const
        {
            if (mUnit < other.mUnit)
                return true;
            if (mUnit > other.mUnit)
                return false;
            return mType < other.mType;
        }*/

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        bool isActive(OffsetPtr offset) const;

        const SerializableUnitBase *unit() const;
    };

    struct META_EXPORT SerializableUnitPtr : SerializableUnitConstPtr {
    private:
        template <typename T>
        SerializableUnitPtr(T *t, std::true_type)
            : SerializableUnitPtr(t ? t->customUnitPtr() : SerializableUnitPtr { nullptr, &serializeTable<decayed_t<T>>() })
        {
        }

        template <typename T>
        SerializableUnitPtr(T *t, std::false_type)
            : SerializableUnitConstPtr(t)
        {
        }

    public:
        constexpr SerializableUnitPtr() = default;

        template <std::derived_from<SerializableUnitBase> T>
            requires(!std::same_as<SerializableUnitBase, T> && !std::is_const_v<T>)
        SerializableUnitPtr(T *t)
            : SerializableUnitPtr(t, has_function_customUnitPtr<T> {})
        {
            static_assert(!std::same_as<T, SyncableUnitBase>);
        }

        constexpr SerializableUnitPtr(const SerializableUnitPtr &other) = default;

        SerializableUnitPtr(SerializableUnitBase *unit, const SerializeTable *type)
            : SerializableUnitConstPtr(unit, type)
        {
        }

        /*template <typename T>
        T *safe_cast() const
        {
            if (!mType->isDerivedFrom<std::remove_const_t<T>>())
                std::terminate();
            return static_cast<T *>(mScope);
        }*/

        explicit operator bool() const
        {
            return mUnit != nullptr;
        }

        StreamResult readState(FormattedSerializeStream &in, const char *name = nullptr, CallerHierarchyBasePtr hierarchy = {}, StateTransmissionFlags flags = 0) const;

        void setSynced(bool b, const CallerHierarchyBasePtr &hierarchy = {}) const;
        void setActive(bool active, bool existenceChanged) const;
        void setParent(SerializableUnitBase *parent) const;

        SerializableUnitBase *unit() const;
    };

}
}