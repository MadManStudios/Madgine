#pragma once

#include "Generic/callerhierarchy.h"
#include "Generic/closure.h"

namespace Engine {
namespace Serialize {

    struct Serializer {
        const char *mFieldName;
        OffsetPtr (*mOffset)() = nullptr;

        void (*mWriteState)(const void *, FormattedSerializeStream &, const char *, CallerHierarchyBasePtr) = nullptr;
        StreamResult (*mReadState)(void *, FormattedSerializeStream &, const char *, CallerHierarchyBasePtr) = nullptr;

        StreamResult (*mReadAction)(void *, FormattedBufferedStream &, PendingRequest &) = nullptr;
        StreamResult (*mReadRequest)(void *, FormattedBufferedStream &, MessageId) = nullptr;

        StreamResult (*mApplySerializableMap)(void *, FormattedSerializeStream &, bool, CallerHierarchyBasePtr) = nullptr;
        void (*mSetDataSynced)(void *, bool, const CallerHierarchyBasePtr &hierarchy) = nullptr;
        void (*mSetActive)(void *, bool, bool) = nullptr;
        void (*mSetParent)(void *) = nullptr;

        void (*mWriteAction)(const void *, const std::set<std::reference_wrapper<FormattedBufferedStream>, CompareStreamId> &outStreams, void *) = nullptr;
        void (*mWriteRequest)(const void *, FormattedBufferedStream &out, void *) = nullptr;

        StreamResult (*mVisitStream)(FormattedSerializeStream &, const char *, const StreamVisitor &) = nullptr;
    };

}
}