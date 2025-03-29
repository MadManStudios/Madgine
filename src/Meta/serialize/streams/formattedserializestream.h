#pragma once

#include "formatter.h"

#include "../primitivetypes.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT FormattedSerializeStream {

        FormattedSerializeStream() = default;
        FormattedSerializeStream(std::unique_ptr<Formatter> format, SerializeStream stream);

        void beginExtendedWrite(const char *name, size_t count);

        void beginCompoundWrite(const char *name);
        void endCompoundWrite(const char *name);

        StreamResult beginExtendedRead(const char *name, size_t count);

        StreamResult beginCompoundRead(const char *name);
        StreamResult endCompoundRead(const char *name);

        void beginContainerWrite(const char *name, uint32_t size = std::numeric_limits<uint32_t>::max());
        void endContainerWrite(const char *name);

        StreamResult beginContainerRead(const char *name, bool sized = true);
        StreamResult endContainerRead(const char *name);

        bool hasContainerItem();

        StreamResult lookupFieldName(std::string &name);

        bool supportsNameLookup() const;

        template <PrimitiveType T>
        void writePrimitive(const T &t, const char *name)
        {
            mFormatter->beginPrimitiveWrite(name, PrimitiveTypeIndex_v<T>);
            mFormatter->write(t);
            mFormatter->endPrimitiveWrite(name, PrimitiveTypeIndex_v<T>);
        }

        template <PrimitiveType T>
        StreamResult readPrimitive(T &t, const char *name)
        {
            STREAM_PROPAGATE_ERROR(mFormatter->beginPrimitiveRead(name, PrimitiveTypeIndex_v<T>));
            STREAM_PROPAGATE_ERROR(mFormatter->read(t));
            return mFormatter->endPrimitiveRead(name, PrimitiveTypeIndex_v<T>);
        }

        explicit operator bool() const;

        SerializeStreamData *data() const;

        bool isMaster(AccessMode mode);

        bool isBinary();

        void setId(ParticipantId id);
        ParticipantId id() const;

        SerializeManager *manager();

        SerializableUnitList &serializableList();

        SerializeStream &stream();

        std::unique_ptr<Formatter> mFormatter;
    };

}
}