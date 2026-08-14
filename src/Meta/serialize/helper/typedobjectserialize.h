#pragma once

#include "../operations.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    StreamResult beginExtendedTypedRead(FormattedSerializeStream &in, T &t, std::span<const char *const> tags)
    {
        if (in.supportsNameLookup()) {
            std::string tag;
            STREAM_PROPAGATE_ERROR(in.lookupFieldName(tag));
            auto p = std::ranges::find(tags, tag);
            if (p == tags.end())
                return STREAM_INTEGRITY_ERROR(in) << "Unknown Tag: " << tag;
            t = static_cast<T>(std::distance(tags.begin(), p));
        } else {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(nullptr, 1));
            STREAM_PROPAGATE_ERROR(read(in, t, "type"));
        }
        return {};
    }

    template <typename T>
    const char *beginExtendedTypedWrite(FormattedSerializeStream &out, const T &t, std::span<const char *const> tags)
    {
        const char *tag = tags[t];
        if (!out.supportsNameLookup()) {
            out.beginExtendedWrite(tag, 1);
            write(out, t, "type");
        }
        return tag;
    }

    inline StreamResult beginExtendedTypedRead(FormattedSerializeStream &in, std::string &tag)
    {
        if (in.supportsNameLookup()) {
            return in.lookupFieldName(tag);
        } else {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead(nullptr, 1));
            return read(in, tag, "type");
        }
    }

    inline const char *beginExtendedTypedWrite(FormattedSerializeStream &out, std::string_view tag)
    {
        const char *cTag = tag.data();
        if (!out.supportsNameLookup()) {
            out.beginExtendedWrite(cTag, 1);
            write(out, tag, "type");
        }
        return cTag;
    }
}
}