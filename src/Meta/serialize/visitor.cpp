#include "../metalib.h"

#include "visitor.h"

#include "streams/streamresult.h"

#include "hierarchy/syncableunit.h"

#include "Generic/bytebuffer.h"
#include "../math/color4.h"
#include "../math/matrix4.h"
#include "../math/quaternion.h"

#include "hierarchy/serializableunitptr.h"

#include "streams/formattedserializestream.h"

#include "../enumholder.h"
#include "../flagsholder.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    StreamResult visitSkipPrimitive(FormattedSerializeStream &in, const char *name)
    {
        T dummy;
        return in.readPrimitive<T>(dummy, name);
    }

    StreamResult visitSkipEnum(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name)
    {
        EnumHolder dummy { table };
        return in.readPrimitive<EnumHolder>(dummy, name);        
    }

    StreamResult visitSkipFlags(const EnumMetaTable *table, FormattedSerializeStream &in, const char *name)
    {
        FlagsHolder dummy { table };
        return in.readPrimitive<FlagsHolder>(dummy, name);        
    }

    StreamResult visitSyncableUnit(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        return SyncableUnitBase::visitStream(table, in, name, visitor);
    }

    template META_EXPORT StreamResult visitSkipPrimitive<bool>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint8_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int8_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint16_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int16_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint32_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int32_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint64_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int64_t>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<float>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SyncableUnitBase *>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SerializableDataPtr>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::string>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<ByteBuffer>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Void>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector2>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector2i>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Quaternion>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector3>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Vector4>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Matrix3>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Color3>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Color4>(FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::chrono::nanoseconds>(FormattedSerializeStream &, const char *);



}
}