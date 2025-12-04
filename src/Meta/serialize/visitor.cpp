#include "../metalib.h"

#include "visitor.h"

#include "Generic/bytebuffer.h"

#include "../enumholder.h"
#include "../flagsholder.h"
#include "../math/color4.h"
#include "../math/matrix4.h"
#include "../math/quaternion.h"
#include "hierarchy/serializableunitptr.h"
#include "hierarchy/syncableunit.h"
#include "streams/formattedserializestream.h"
#include "streams/streamresult.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    StreamResult visitSkipPrimitive(CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        T dummy;
        return in.mStream.readPrimitive<T>(dummy, name);
    }

    StreamResult visitSkipEnum(const EnumMetaTable *table, CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        EnumHolder dummy { table };
        return in.mStream.readPrimitive<EnumHolder>(dummy, name);
    }

    StreamResult visitSkipFlags(const EnumMetaTable *table, CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        FlagsHolder dummy { table };
        return in.mStream.readPrimitive<FlagsHolder>(dummy, name);
    }

    StreamResult visitSyncableUnit(const SerializeTable *table, CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return SyncableUnitBase::visitStream(table, in, name, visitor, depth);
    }

    template META_EXPORT StreamResult visitSkipPrimitive<bool>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint8_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int8_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint16_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int16_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint32_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int32_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint64_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int64_t>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<float>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SyncableUnitBase *>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SerializableDataPtr>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::string>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<ByteBuffer>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Void>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector2>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector2i>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Quaternion>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector3>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector4>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Matrix3>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Color3>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Color4>(CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::chrono::nanoseconds>(CallerHierarchyFormattedSerializeStream, const char *);

}
}