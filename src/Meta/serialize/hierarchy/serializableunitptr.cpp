#include "../../metalib.h"

#include "statetransmissionflags.h"

#include "serializetable.h"

#include "Generic/offsetptr.h"

#include "toplevelunit.h"

#include "../operations.h"

namespace Engine {
namespace Serialize {

    void SerializableDataConstPtr::writeState(FormattedSerializeStream &out, const char *name, CallerHierarchyBasePtr hierarchy, bool skipId) const
    {
        if (out.isMaster(AccessMode::WRITE) && out.data() && !skipId) {
            out.beginExtendedWrite(name, 1);
            Serialize::write(out, *this, "serId");
        }

        out.beginCompoundWrite(name);
        mType->writeState(mUnit, out, hierarchy);
        out.endCompoundWrite(name);
    }

    StreamResult SerializableDataPtr::readState(FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy, bool skipId) const
    {
        if (!in.isMaster(AccessMode::READ) && in.data() && !skipId) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            SerializableUnitBase *idHelper;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, idHelper, "serId"));
            uint32_t id = reinterpret_cast<uintptr_t>(idHelper) >> 2;
            SerializableUnitList &list = in.serializableList();
            if (list.size() <= id)
                list.resize(id + 1);
            assert(!list[id]);
            list[id] = *this;
        }

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(mType->readState(unit(), in, hierarchy));
        return in.endCompoundRead(name);
    }

    void SerializableDataPtr::setActive(bool active, bool existenceChanged) const
    {
        // assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged);
    }

    StreamResult SerializableDataPtr::visitStream(const SerializeTable *type, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        assert(!in.isMaster(AccessMode::READ));

        STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
        uint32_t idHelper;
        STREAM_PROPAGATE_ERROR(read(in, idHelper, "serId"));

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(type->visitStream(in, visitor));
        return in.endCompoundRead(name);
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

    StreamResult SerializableUnitPtr::readState(FormattedSerializeStream &in, const char *name, CallerHierarchyBasePtr hierarchy, bool skipId) const
    {
        return SerializableDataPtr { unit(), mType }.readState(in, name, hierarchy, skipId);
    }

    void SerializableUnitPtr::setParent(SerializableUnitBase *parent) const
    {
        if (unit()->mTopLevel != mUnit)
            unit()->mTopLevel = parent ? parent->mTopLevel : nullptr;
        mType->setParent(unit());
    }

    StreamResult SerializableDataPtr::applyMap(FormattedSerializeStream &in, bool success, CallerHierarchyBasePtr hierarchy) const
    {
        return mType->applyMap(unit(), in, success, hierarchy);
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