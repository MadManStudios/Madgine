#include "../../metalib.h"

#include "serializableunit.h"

namespace Engine {
namespace Serialize {

    SerializableUnitBase::SerializableUnitBase()
    {
    }

    SerializableUnitBase::SerializableUnitBase(const SerializableUnitBase &other)
        : SerializableUnitBase()
    {
    }

    SerializableUnitBase::SerializableUnitBase(SerializableUnitBase &&other) noexcept
        : mTopLevel(std::exchange(other.mTopLevel, nullptr))
        , mActiveIndex(std::exchange(other.mActiveIndex, 0))
        , mSynced(std::exchange(other.mSynced, false))
    {
    }

    SerializableUnitBase::~SerializableUnitBase()
    {
        // If this fails, you forgot to add all Serializables to the SerializeTable.
        // Temporary solution! Proper solution: Move mSynced into Serializable (=> evtl default mActive to true)
        assert(!mSynced);
    }

    SerializableUnitBase &SerializableUnitBase::operator=(const SerializableUnitBase &other)
    {
        return *this;
    }

    SerializableUnitBase &SerializableUnitBase::operator=(SerializableUnitBase &&other)
    {
        mTopLevel = std::exchange(other.mTopLevel, nullptr);
        mActiveIndex = std::exchange(other.mActiveIndex, 0);
        std::exchange(mSynced, other.mSynced);
        return *this;
    }

    bool SerializableUnitBase::isSynced() const
    {
        return mSynced;
    }

} // namespace Serialize
} // namespace Core
