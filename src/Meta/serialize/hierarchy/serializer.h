#pragma once

#include "Generic/callerhierarchy.h"
#include "Generic/closure.h"
#include "Generic/offsetptr.h"

namespace Engine {
namespace Serialize {

    struct Serializer {
        const char *mFieldName;
        OffsetPtr mOffset;

        void (*mWriteState)(const void *, CallerHierarchyFormattedSerializeStream, const char *) = nullptr;
        StreamResult (*mReadState)(void *, CallerHierarchyFormattedSerializeStream, const char *) = nullptr;

        StreamResult (*mReadAction)(void *, CallerHierarchyFormattedSerializeStream, PendingRequest &) = nullptr;
        StreamResult (*mReadRequest)(void *, FormattedMessageStream &, MessageId) = nullptr;

        StreamResult (*mApplySerializableMap)(const Serializer *, void *, CallerHierarchyFormattedSerializeStream, bool) = nullptr;
        void (*mSetDataSynced)(const Serialize::Serializer *serializer, void *, bool, const CallerHierarchyBasePtr &hierarchy) = nullptr;
        void (*mSetActive)(const Serialize::Serializer *serializer, void *, bool, bool) = nullptr;
        void (*mSetParent)(const Serialize::Serializer *serializer, void *) = nullptr;

        void (*mWriteAction)(const void *, const std::vector<WriteMessage> &outStreams, void *) = nullptr;
        void (*mWriteRequest)(const void *, CallerHierarchyFormattedSerializeStream out, void *) = nullptr;

        StreamResult (*mVisitStream)(CallerHierarchyFormattedSerializeStream, const char *, const StreamVisitor &, size_t) = nullptr;
    };

}
}