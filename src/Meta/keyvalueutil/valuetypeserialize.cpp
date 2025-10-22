#include "../metalib.h"

#include "valuetypeserialize.h"

#include "../serialize/streams/serializestream.h"

#include "../keyvalue/valuetype.h"

#include "../serialize/streams/formattedserializestream.h"

#include "Meta/serialize/operations.h"

namespace Engine {
namespace Serialize {

    StreamResult Operations<ValueType>::read(CallerHierarchyFormattedSerializeStream in, ValueType &v, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
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

    void Operations<ValueType>::write(CallerHierarchyFormattedSerializeStream out, const ValueType &v, const char *name)    
    {
        out.mStream.beginExtendedWrite(name, 1);
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

    StreamResult Operations<ValueType>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
        ValueTypeEnum type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "type"));
        ValueType v;
        v.setType(ValueTypeDesc { type });
        return v.visit([&](auto &value) -> StreamResult {
            using T = std::remove_reference_t<decltype(value)>;
            if constexpr (PrimitiveType<T>) {
                return Serialize::visitStream<T>(in, name, visitor, depth);
            } else if constexpr (std::same_as<T, std::monostate>) {                
                return Serialize::visitStream<Void>(in, name, visitor, depth);
            } else
                throw 0;
        });
    }

    
    StreamResult Operations<ExtendedValueTypeDesc>::read(CallerHierarchyFormattedSerializeStream in, ExtendedValueTypeDesc &t, const char *name)
    {
        std::string type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, name));
        //TODO
        return {};
    }

    void Operations<ExtendedValueTypeDesc>::write(CallerHierarchyFormattedSerializeStream out, const ExtendedValueTypeDesc &t, const char *name)
    {
        Serialize::write(out, t.toString(), name);
    }

    StreamResult Operations<ExtendedValueTypeDesc>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return Serialize::visitStream<std::string>(in, name, visitor, depth);
    }
}
}
