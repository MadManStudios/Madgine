#include "../../metalib.h"

#include "Generic/offsetptr.h"

#include "../operations.h"
#include "serializetable.h"
#include "toplevelunit.h"

namespace Engine {
namespace Serialize {

    void SerializableDataConstPtr::writeState(CallerHierarchyFormattedSerializeStream out, const char *name, bool skipId) const
    {
        if (out.mStream.isMaster(AccessMode::WRITE) && out.mStream.data() && !skipId) {
            out.mStream.beginExtendedWrite(name, 1);
            Serialize::write(out, *this, "serId");
        }

        out.mStream.beginCompoundWrite(name);
        mType->writeState(mUnit, out);
        out.mStream.endCompoundWrite(name);
    }

    StreamResult SerializableDataPtr::readState(CallerHierarchyFormattedSerializeStream in, const char *name, bool skipId) const
    {
        if (!in.mStream.isMaster(AccessMode::READ) && in.mStream.data() && !skipId) {
            STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
            SerializableUnitBase *idHelper;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, idHelper, "serId"));
            uint32_t id = reinterpret_cast<uintptr_t>(idHelper) >> 2;
            SerializableUnitList &list = in.mStream.serializableList();
            if (list.size() <= id)
                list.resize(id + 1);
            assert(!list[id]);
            list[id] = *this;
        }

        STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(mType->readState(unit(), in));
        return in.mStream.endCompoundRead(name);
    }

    void SerializableDataPtr::setActive(bool active, bool existenceChanged) const
    {
        // assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    StreamResult SerializableDataPtr::visitStream(const SerializeTable *type, CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        assert(!in.mStream.isMaster(AccessMode::READ));

        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
        uint32_t idHelper;
        STREAM_PROPAGATE_ERROR(read(in, idHelper, "serId"));

        STREAM_PROPAGATE_ERROR(in.mStream.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(type->visitStream(in, visitor, depth));
        return in.mStream.endCompoundRead(name);
    }

    SerializableDataPtr::SerializableDataPtr(const SerializableUnitPtr &other)
        : SerializableDataPtr(other.unit(), other.mType)
    {
    }

    void *SerializableDataPtr::unit() const
    {
        return const_cast<void *>(mUnit);
    }

    const SerializableUnitBase *SerializableUnitConstPtr::unit() const
    {
        return static_cast<const SerializableUnitBase *>(mUnit);
    }

    SerializableUnitConstPtr::SerializableUnitConstPtr(const SerializableUnitBase *unit, const SerializeTable *type)
        : SerializableDataConstPtr { unit, type }
    {
    }

    bool SerializableUnitConstPtr::isActive(OffsetPtr offset) const
    {
        // TODO: Maybe save lookup -> enforce order of elements in memory
        return mType->getIndex(offset) < unit()->mActiveIndex;
    }

    StreamResult SerializableUnitPtr::readState(CallerHierarchyFormattedSerializeStream in, const char *name, bool skipId) const
    {
        return SerializableDataPtr { unit(), mType }.readState(in, name, skipId);
    }

    void SerializableUnitPtr::setParent(SerializableUnitBase *parent) const
    {
        if (unit()->mTopLevel != mUnit)
            unit()->mTopLevel = parent ? parent->mTopLevel : nullptr;
        mType->setParent(unit());
    }

    StreamResult SerializableDataPtr::applyMap(CallerHierarchyFormattedSerializeStream in, bool success) const
    {
        return mType->applyMap(unit(), in, success);
    }

    void SerializableUnitPtr::setSynced(bool b, const CallerHierarchyBasePtr &hierarchy) const
    {
        assert(unit()->mSynced != b);
        unit()->mSynced = b;
        mType->setSynced(unit(), b, hierarchy);
    }

    void SerializableUnitPtr::setActive(bool active, bool existenceChanged) const
    {
        // assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    SerializableUnitBase *SerializableUnitPtr::unit() const
    {
        return const_cast<SerializableUnitBase *>(SerializableUnitConstPtr::unit());
    }
}
}