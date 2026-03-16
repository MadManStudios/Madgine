#pragma once

#include "Generic/genericresult.h"

namespace Engine {

struct META_EXPORT KeyValueError {
    KeyValueError(GenericResult state = GenericResult::UNKNOWN_ERROR, const std::string &msg = "");
    KeyValueError(GenericResult state, const std::string &msg, const char *function, const char *file, size_t sourceLine);

    META_EXPORT friend std::ostream &operator<<(std::ostream &out, const KeyValueError &error);

    std::string mMsg;
    struct StackEntry {
        std::string mFunction;
        std::string mFile;
        size_t mLineNr;
    };
    std::vector<StackEntry> mStackTrace;

    GenericResult mState;
};

struct [[nodiscard]] META_EXPORT KeyValueResult {

    KeyValueResult() = default;
    KeyValueResult(std::unique_ptr<KeyValueError> error);
    KeyValueResult(const KeyValueResult &other);
    KeyValueResult(KeyValueResult &&) = default;

    KeyValueResult &operator=(const KeyValueResult &) = delete;
    KeyValueResult &operator=(KeyValueResult &&) = default;
    
    explicit operator bool() const;
        
    std::unique_ptr<KeyValueError> mError;

    META_EXPORT friend std::ostream &operator<<(std::ostream &out, const KeyValueResult &result);
};

struct META_EXPORT KeyValueResultBuilder {
    GenericResult mType;
    const char *mFunction;
    const char *mFile;
    size_t mLine;
    std::ostringstream mMsg;

    KeyValueResultBuilder(GenericResult type, const char *function, const char *file, size_t line)
        : mType(type)
        , mFunction(function)
        , mFile(file)
        , mLine(line)
    {
    }

    operator KeyValueResult();
    operator KeyValueError();

    template <typename T>
    KeyValueResultBuilder &&operator<<(T &&t) &&
    {
        mMsg << std::forward<T>(t);
        return std::move(*this);
    }
};

#define KEYVALUE_ERROR(Type) \
    ::Engine::KeyValueResultBuilder { Type, __func__, __FILE__, __LINE__ }

#define KEYVALUE_UNKNOWN_ERROR() KEYVALUE_ERROR(::Engine::GenericResult::UNKNOWN_ERROR)

#define KEYVALUE_PROPAGATE_ERROR(...)                                                                                        \
    if (::Engine::KeyValueResult _result = (__VA_ARGS__)) \
    return _result

}