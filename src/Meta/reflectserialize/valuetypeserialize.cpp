#include "../metalib.h"

#include "valuetypeserialize.h"

#include "Meta/serialize/operations.h"

#include "../reflect/value.h"
#include "../serialize/streams/formattedserializestream.h"
#include "../serialize/streams/serializestream.h"

namespace Engine {
namespace Serialize {

    StreamResult Operations<Reflect::Value>::read(CallerHierarchyFormattedSerializeStream in, Reflect::Value &v, const char *name)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
        Reflect::TypeEnum type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "type"));
        v.setType(Reflect::Type { type });
        return v.visit([&](auto &value) -> StreamResult {
            using T = std::remove_reference_t<decltype(value)>;
            if constexpr (PrimitiveType<T> || Concepts::InstanceOf<T, std::chrono::duration>) {
                return Serialize::read(in, value, name);
            } else if constexpr (std::same_as<T, std::monostate>) {
                Void v;
                return Serialize::read(in, v, name);
            } else
                throw 0;
        });
    }

    void Operations<Reflect::Value>::write(CallerHierarchyFormattedSerializeStream out, const Reflect::Value &v, const char *name)
    {
        out.mStream.beginExtendedWrite(name, 1);
        Serialize::write(out, v.index().mIndex, "type");
        v.visit([&](const auto &value) {
            using T = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            if constexpr (PrimitiveType<T> || Concepts::InstanceOf<T, std::chrono::duration>) {
                Serialize::write(out, value, name);
            } else if constexpr (std::same_as<T, std::monostate>) {
                Serialize::write(out, Void {}, name);
            } else
                throw 0;
        });
    }

    StreamResult Operations<Reflect::Value>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(name, 1));
        Reflect::TypeEnum type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, "type"));
        Reflect::Value v;
        v.setType(Reflect::Type { type });
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

    StreamResult Operations<Reflect::ExtendedType>::read(CallerHierarchyFormattedSerializeStream in, Reflect::ExtendedType &t, const char *name)
    {
        std::string type;
        STREAM_PROPAGATE_ERROR(Serialize::read(in, type, name));
        // TODO
        return {};
    }

    void Operations<Reflect::ExtendedType>::write(CallerHierarchyFormattedSerializeStream out, const Reflect::ExtendedType &t, const char *name)
    {
        Serialize::write(out, t.toString(), name);
    }

    StreamResult Operations<Reflect::ExtendedType>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        return Serialize::visitStream<std::string>(in, name, visitor, depth);
    }
}
}
