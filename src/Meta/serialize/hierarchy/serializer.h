#pragma once

#include "Generic/closure.h"
#include "Generic/offsetptr.h"

#include "../context.h"

namespace Engine {
namespace Serialize {

    struct Serializer {
        const char *mFieldName;
        OffsetPtr mOffset;

        void (*mWriteState)(const void *, FormattedSerializeStream &, const char *, ContextPtr) = nullptr;
        StreamResult (*mReadState)(void *, FormattedSerializeStream &, const char *, ContextPtr) = nullptr;

        StreamResult (*mReadAction)(void *, FormattedSerializeStream &, PendingRequest &, ContextPtr) = nullptr;
        StreamResult (*mReadRequest)(void *, FormattedMessageStream &, MessageId) = nullptr;

        StreamResult (*mApplySerializableMap)(const Serializer *, void *, FormattedSerializeStream &, bool, ContextPtr context) = nullptr;
        void (*mSetDataSynced)(const Serialize::Serializer *serializer, void *, bool, ContextPtr) = nullptr;
        void (*mSetActive)(const Serialize::Serializer *serializer, void *, bool, bool, ContextPtr) = nullptr;
        void (*mSetParent)(const Serialize::Serializer *serializer, void *) = nullptr;

        void (*mWriteAction)(const void *, const std::vector<WriteMessage> &outStreams, void *) = nullptr;
        void (*mWriteRequest)(const void *, FormattedSerializeStream &out, void *, ContextPtr) = nullptr;

        StreamResult (*mVisitStream)(FormattedSerializeStream &, const char *, const StreamVisitor &, size_t) = nullptr;
    };

}
}