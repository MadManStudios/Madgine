#include "../metalib.h"

#include "visitor.h"

#include "Generic/bytebuffer.h"

#include "../math/color4.h"
#include "../math/matrix4.h"
#include "../math/quaternion.h"
#include "../reflect/enum.h"
#include "../reflect/flags.h"
#include "../reflectserialize/valuetypeserialize.h"
#include "hierarchy/serializableunitptr.h"
#include "hierarchy/syncableunit.h"
#include "streams/formattedserializestream.h"
#include "streams/streamresult.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    StreamResult visitSkipPrimitive(PrimitiveHolder<T>, CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        T dummy;
        return in.mStream.readPrimitive<T>(dummy, name);
    }

    template <>
    META_EXPORT StreamResult visitSkipPrimitive(PrimitiveHolder<EnumTag> holder, CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        Reflect::Enum dummy { holder.mTable };
        return in.mStream.readPrimitive<Reflect::Enum>(dummy, name);
    }

    template <>
    META_EXPORT StreamResult visitSkipPrimitive(PrimitiveHolder<FlagsTag> holder, CallerHierarchyFormattedSerializeStream in, const char *name)
    {
        Reflect::Flags dummy { holder.mTable };
        return in.mStream.readPrimitive<Reflect::Flags>(dummy, name);
    }

    StreamResult visitSyncableUnit(const SerializeTable *table, CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return SyncableUnitBase::visitStream(table, in, name, visitor, depth);
    }

    template META_EXPORT StreamResult visitSkipPrimitive<bool>(PrimitiveHolder<bool>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint8_t>(PrimitiveHolder<uint8_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int8_t>(PrimitiveHolder<int8_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint16_t>(PrimitiveHolder<uint16_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int16_t>(PrimitiveHolder<int16_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint32_t>(PrimitiveHolder<uint32_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int32_t>(PrimitiveHolder<int32_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint64_t>(PrimitiveHolder<uint64_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int64_t>(PrimitiveHolder<int64_t>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<float>(PrimitiveHolder<float>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SyncableUnitBase *>(PrimitiveHolder<SyncableUnitBase *>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SerializableDataPtr>(PrimitiveHolder<SerializableDataPtr>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::string>(PrimitiveHolder<std::string>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Memory::ByteBuffer>(PrimitiveHolder<Memory::ByteBuffer>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::monostate>(PrimitiveHolder<std::monostate>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector2>(PrimitiveHolder<Math::Vector2>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector2i>(PrimitiveHolder<Math::Vector2i>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Quaternion>(PrimitiveHolder<Math::Quaternion>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector3>(PrimitiveHolder<Math::Vector3>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector4>(PrimitiveHolder<Math::Vector4>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Matrix3>(PrimitiveHolder<Math::Matrix3>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Matrix4>(PrimitiveHolder<Math::Matrix4>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Color3>(PrimitiveHolder<Math::Color3>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Color4>(PrimitiveHolder<Math::Color4>, CallerHierarchyFormattedSerializeStream, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::chrono::nanoseconds>(PrimitiveHolder<std::chrono::nanoseconds>, CallerHierarchyFormattedSerializeStream, const char *);

}
}