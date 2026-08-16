#include "../../metalib.h"

#include "Generic/offsetptr.h"

#include "../operations.h"
#include "serializetable.h"
#include "toplevelunit.h"

namespace Engine {
namespace Serialize {

    void SerializableDataConstPtr::writeState(FormattedSerializeStream &out, const char *name, bool skipId, ContextPtr context) const
    {
        if (out.isMaster(AccessMode::WRITE) && out.data() && !skipId) {
            out.beginExtendedWrite(name, 1);
            Serialize::write(out, *this, "serId");
        }

        out.beginCompoundWrite(name);
        mType->writeState(mUnit, out, context);
        out.endCompoundWrite(name);
    }

    StreamResult SerializableDataPtr::readState(FormattedSerializeStream &in, const char *name, bool skipId, ContextPtr context) const
    {
        if (!in.isMaster(AccessMode::READ) && in.data() && !skipId) {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
            SerializableUnitBase *idHelper;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, idHelper, "serId", context));
            uint32_t id = reinterpret_cast<uintptr_t>(idHelper) >> 2;
            SerializableUnitList &list = in.serializableList();
            if (list.size() <= id)
                list.resize(id + 1);
            assert(!list[id] || list[id] == *this);
            list[id] = *this;
        }

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(mType->readState(unit(), in, context));
        return in.endCompoundRead(name);
    }

    StreamResult SerializableDataPtr::applyMap(FormattedSerializeStream &in, bool success, ContextPtr context) const
    {
        return mType->applyMap(unit(), in, success, context);
    }

    void SerializableDataPtr::setActive(bool active, bool existenceChanged, ContextPtr context) const
    {
        // assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged, context);
    }

    StreamResult SerializableDataPtr::visitStream(const SerializeTable *type, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        assert(!in.isMaster(AccessMode::READ));

        STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
        uint32_t idHelper;
        STREAM_PROPAGATE_ERROR(read(in, idHelper, "serId"));

        STREAM_PROPAGATE_ERROR(in.beginCompoundRead(name));
        STREAM_PROPAGATE_ERROR(type->visitStream(in, visitor, depth));
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

    StreamResult SerializableUnitPtr::readState(FormattedSerializeStream &in, const char *name, bool skipId, ContextPtr context) const
    {
        return SerializableDataPtr { unit(), mType }.readState(in, name, skipId, context);
    }

    void SerializableUnitPtr::setParent(SerializableUnitBase *parent) const
    {
        if (unit()->mTopLevel != mUnit)
            unit()->mTopLevel = parent ? parent->mTopLevel : nullptr;
        mType->setParent(unit());
    }

    void SerializableUnitPtr::setSynced(bool b, ContextPtr context) const
    {
        assert(unit()->mSynced != b);
        unit()->mSynced = b;
        mType->setSynced(unit(), b, context);
    }

    void SerializableUnitPtr::setActive(bool active, bool existenceChanged, ContextPtr context) const
    {
        // assert(mSynced == active);
        mType->setActive(unit(), active, existenceChanged, context);
    }

    SerializableUnitBase *SerializableUnitPtr::unit() const
    {
        return const_cast<SerializableUnitBase *>(SerializableUnitConstPtr::unit());
    }

    StreamResult SerializableUnitPtr::applyMap(FormattedSerializeStream &in, bool success, ContextPtr context) const
    {
        return mType->applyMap(unit(), in, success, context);
    }
}
}