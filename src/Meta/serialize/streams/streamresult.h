#pragma once

#include "streamstate.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT StreamError {
        StreamError(SerializeStream *in, bool binary, const std::string &msg, const char *file, size_t sourceLine);

        META_EXPORT friend std::ostream &operator<<(std::ostream &out, const StreamError &error);

        int mLineNumber = 0, mPosition = 0;
        std::string mMsg;
        std::string mNotes;
    };

    struct [[nodiscard]] StreamResult {
        StreamState mState = StreamState::OK;
        std::unique_ptr<StreamError> mError;

        META_EXPORT friend std::ostream &operator<<(std::ostream &out, const StreamResult &result);
    };

    struct META_EXPORT StreamResultBuilder {
        StreamState mType;
        SerializeStream *mStream = nullptr;
        bool mBinary;
        const char *mFile;
        size_t mLine;
        std::ostringstream mMsg;

        StreamResultBuilder(StreamState type, SerializeStream &stream, bool binary, const char *file, size_t line)
            : mType(type)
            , mStream(&stream)
            , mFile(file)
            , mLine(line)
        {
        }

        StreamResultBuilder(StreamState type, FormattedSerializeStream &stream, const char *file, size_t line);

        StreamResultBuilder(StreamState type, const char *file, size_t line)
            : mType(type)
            , mFile(file)
            , mLine(line)
        {
        }

        operator StreamResult();

        template <typename T>
        StreamResultBuilder &&operator<<(T &&t) &&
        {
            mMsg << std::forward<T>(t);
            return std::move(*this);
        }
    };

#if _MSC_VER && CLANG
#    define STREAM_ERROR(Type, ...) \
        ::Engine::Serialize::StreamResultBuilder { Type, __VA_ARGS__, __FILE__, __LINE__ }
#else
#    define STREAM_ERROR(Type, ...) \
        ::Engine::Serialize::StreamResultBuilder { Type, __VA_ARGS__ __VA_OPT__(, ) __FILE__, __LINE__ }
#endif
#define STREAM_PARSE_ERROR(...) STREAM_ERROR(::Engine::Serialize::StreamState::PARSE_ERROR, __VA_ARGS__)
#define STREAM_PERMISSION_ERROR(...) STREAM_ERROR(::Engine::Serialize::StreamState::PERMISSION_ERROR, __VA_ARGS__)
#define STREAM_INTEGRITY_ERROR(...) STREAM_ERROR(::Engine::Serialize::StreamState::INTEGRITY_ERROR, __VA_ARGS__)
#define STREAM_CONNECTION_LOST_ERROR(...) STREAM_ERROR(::Engine::Serialize::StreamState::CONNECTION_LOST, __VA_ARGS__)
#define STREAM_UNKNOWN_ERROR(...) STREAM_ERROR(::Engine::Serialize::StreamState::UNKNOWN_ERROR, __VA_ARGS__)

#define STREAM_PROPAGATE_ERROR(...)                                                                                        \
    if (::Engine::Serialize::StreamResult _result = (__VA_ARGS__); _result.mState != ::Engine::Serialize::StreamState::OK) \
    return _result

    META_EXPORT StreamState streamError(std::ios::iostate state, std::ostringstream &out);

}
}