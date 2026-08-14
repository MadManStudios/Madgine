#include "../metalib.h"

#include "visitor.h"

#include "Generic/bytebuffer.h"

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
    StreamResult visitSkipPrimitive(PrimitiveHolder<T>, FormattedSerializeStream &in, const char *name)
    {
        T dummy;
        return in.readPrimitive<T>(dummy, name);
    }

    template <>
    META_EXPORT StreamResult visitSkipPrimitive(PrimitiveHolder<EnumTag> holder, FormattedSerializeStream &in, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mFormatter->beginPrimitiveRead(name, PrimitiveTypeIndex_v<EnumTag>));
        int32_t v;
        if (in.isBinary()) {            
            STREAM_PROPAGATE_ERROR(in.mFormatter->stream().read(v));
        } else {
            if (!holder.mTable->read(in.mFormatter->stream().stream(), v, holder.mTable->mName)) {
                auto error = STREAM_ERROR(StreamState::OK, in.mFormatter->stream(), false);
                error.mType = streamError(in.mFormatter->stream().state(), error.mMsg);
                error.mMsg << "after read";
                return error;
            }
        }
        return in.mFormatter->endPrimitiveRead(name, PrimitiveTypeIndex_v<EnumTag>);
    }

    template <>
    META_EXPORT StreamResult visitSkipPrimitive(PrimitiveHolder<FlagsTag> holder, FormattedSerializeStream &in, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mFormatter->beginPrimitiveRead(name, PrimitiveTypeIndex_v<FlagsTag>));
        uint64_t v;
        if (in.isBinary()) {
            STREAM_PROPAGATE_ERROR(in.mFormatter->stream().read(v));
        } else {
            if (!holder.mTable->readFlags(in.mFormatter->stream().stream(), v)) {
                auto error = STREAM_ERROR(StreamState::OK, in.mFormatter->stream(), false);
                error.mType = streamError(in.mFormatter->stream().state(), error.mMsg);
                error.mMsg << "after read";
                return error;
            }
        }
        return in.mFormatter->endPrimitiveRead(name, PrimitiveTypeIndex_v<EnumTag>);
    }

    StreamResult visitSyncableUnit(const SerializeTable *table, FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return SyncableUnitBase::visitStream(table, in, name, visitor, depth);
    }

    template META_EXPORT StreamResult visitSkipPrimitive<bool>(PrimitiveHolder<bool>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint8_t>(PrimitiveHolder<uint8_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int8_t>(PrimitiveHolder<int8_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint16_t>(PrimitiveHolder<uint16_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int16_t>(PrimitiveHolder<int16_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint32_t>(PrimitiveHolder<uint32_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int32_t>(PrimitiveHolder<int32_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<uint64_t>(PrimitiveHolder<uint64_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<int64_t>(PrimitiveHolder<int64_t>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<float>(PrimitiveHolder<float>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SyncableUnitBase *>(PrimitiveHolder<SyncableUnitBase *>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<SerializableDataPtr>(PrimitiveHolder<SerializableDataPtr>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::string>(PrimitiveHolder<std::string>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Memory::ByteBuffer>(PrimitiveHolder<Memory::ByteBuffer>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::monostate>(PrimitiveHolder<std::monostate>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector2>(PrimitiveHolder<Math::Vector2>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector2i>(PrimitiveHolder<Math::Vector2i>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Quaternion>(PrimitiveHolder<Math::Quaternion>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector3>(PrimitiveHolder<Math::Vector3>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Vector4>(PrimitiveHolder<Math::Vector4>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Matrix3>(PrimitiveHolder<Math::Matrix3>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Matrix4>(PrimitiveHolder<Math::Matrix4>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Color3>(PrimitiveHolder<Math::Color3>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<Math::Color4>(PrimitiveHolder<Math::Color4>, FormattedSerializeStream &, const char *);
    template META_EXPORT StreamResult visitSkipPrimitive<std::chrono::nanoseconds>(PrimitiveHolder<std::chrono::nanoseconds>, FormattedSerializeStream &, const char *);

}
}