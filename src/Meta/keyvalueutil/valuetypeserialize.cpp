#include "../metalib.h"

#include "valuetypeserialize.h"

#include "../serialize/streams/serializestream.h"

#include "../keyvalue/valuetype.h"

#include "../serialize/streams/formattedserializestream.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Serialize {

    StreamResult Operations<ValueType>::read(FormattedSerializeStream &in, ValueType &v, const char *name, const CallerHierarchyBasePtr &hierarchy)
    {
        STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
        ValueTypeEnum type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "type"));
        v.setType(ValueTypeDesc { type });
        return v.visit([&](auto &value) -> StreamResult {
            using T = std::remove_reference_t<decltype(value)>;
            if constexpr (PrimitiveType<T> || InstanceOf<T, std::chrono::duration>) {
                return Serialize::read(in, value, name);
            } else if constexpr (std::same_as<T, std::monostate>) {
                Void v;
                return Serialize::read(in, v, name);
            } else
                throw 0;
        });
    }

    void Operations<ValueType>::write(FormattedSerializeStream &out, const ValueType &v, const char *name, const CallerHierarchyBasePtr &hierarchy)    
    {
        out.beginExtendedWrite(name, 1);
        Serialize::write(out, v.index().mIndex, "type");
        v.visit([&](const auto &value) {
            using T = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            if constexpr (PrimitiveType<T> || InstanceOf<T, std::chrono::duration>) {
                Serialize::write(out, value, name);
            } else if constexpr (std::same_as<T, std::monostate>){
                Serialize::write(out, Void {}, name);
            } else
                throw 0;
        });
    }

    StreamResult Operations<ValueType>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        STREAM_PROPAGATE_ERROR(in.beginExtendedRead(name, 1));
        ValueTypeEnum type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "type"));
        ValueType v;
        v.setType(ValueTypeDesc { type });
        return v.visit([&](auto &value) -> StreamResult {
            using T = std::remove_reference_t<decltype(value)>;
            if constexpr (PrimitiveType<T>) {
                return Serialize::visitStream<T>(in, name, visitor);
            } else if constexpr (std::same_as<T, std::monostate>) {                
                return Serialize::visitStream<Void>(in, name, visitor);
            } else
                throw 0;
        });
    }

    
    StreamResult Operations<ExtendedValueTypeDesc>::read(FormattedSerializeStream &in, ExtendedValueTypeDesc &t, const char *name, const CallerHierarchyBasePtr &hierarchy)
    {
        std::string type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, name, hierarchy));
        return {};
    }

    void Operations<ExtendedValueTypeDesc>::write(FormattedSerializeStream &out, const ExtendedValueTypeDesc &t, const char *name, const CallerHierarchyBasePtr &hierarchy)
    {
        Serialize::write(out, t.toString(), name, hierarchy);
    }

    StreamResult Operations<ExtendedValueTypeDesc>::applyMap(FormattedSerializeStream &in, ExtendedValueTypeDesc &t, bool success, const CallerHierarchyBasePtr &hierarchy)
    {
        return {};
    }

    StreamResult Operations<ExtendedValueTypeDesc>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
    {
        return Serialize::visitStream<std::string>(in, name, visitor);
    }
}
}
