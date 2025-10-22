#pragma once

#include "../operations.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    StreamResult beginExtendedTypedRead(CallerHierarchyFormattedSerializeStream in, T &t, std::span<const char *const> tags)
    {
        if (in.mStream.supportsNameLookup()) {
            std::string tag;
            STREAM_PROPAGATE_ERROR(in.mStream.lookupFieldName(tag));
            auto p = std::ranges::find(tags, tag);
            if (p == tags.end())
                return STREAM_INTEGRITY_ERROR(in) << "Unknown Tag: " << tag;
            t = static_cast<T>(std::distance(tags.begin(), p));
        } else {
            STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(nullptr, 1));
            STREAM_PROPAGATE_ERROR(read(in, t, "type"));
        }
        return {};
    }

    template <typename T>
    const char *beginExtendedTypedWrite(CallerHierarchyFormattedSerializeStream out, const T &t, std::span<const char *const> tags)
    {
        const char *tag = tags[t];
        if (!out.mStream.supportsNameLookup()) {
            out.mStream.beginExtendedWrite(tag, 1);
            write(out, t, "type");
        }
        return tag;
    }

    inline StreamResult beginExtendedTypedRead(CallerHierarchyFormattedSerializeStream in, std::string &tag)
    {
        if (in.mStream.supportsNameLookup()) {
            return in.mStream.lookupFieldName(tag);
        } else {
            STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead(nullptr, 1));
            return read(in, tag, "type");
        }
    }

    inline const char *beginExtendedTypedWrite(CallerHierarchyFormattedSerializeStream out, std::string_view tag)
    {
        const char *cTag = tag.data();
        if (!out.mStream.supportsNameLookup()) {
            out.mStream.beginExtendedWrite(cTag, 1);
            write(out, tag, "type");
        }
        return cTag;
    }
}
}