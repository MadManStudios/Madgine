#pragma once

#include "Generic/genericresult.h"

#include "enum.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT Error {
        struct StackEntry {
            std::string mFunction;
            std::string mFile;
            size_t mLineNr;
        };

        template <typename Rep, typename... Reps>
        Error(EnumImpl<Rep, Reps...> state, const std::string &msg = "")
            : Error(Enum { state }, msg)
        {
        }
        Error(Enum state = GenericResult { GenericResult::UNKNOWN_ERROR }, const std::string &msg = "");
        Error(Enum state, const std::string &msg, const char *function, const char *file, size_t sourceLine);
        Error(Enum state, const std::string &msg, std::vector<StackEntry> stack);

        META_EXPORT friend std::ostream &operator<<(std::ostream &out, const Error &error);

        std::string mMsg;
        std::vector<StackEntry> mStackTrace;

        Enum mState;
    };

    struct [[nodiscard]] META_EXPORT Result {

        Result() = default;
        Result(std::unique_ptr<Error> error);
        Result(const Result &other);
        Result(Result &&) = default;

        Result &operator=(const Result &) = delete;
        Result &operator=(Result &&) = default;

        explicit operator bool() const;

        std::unique_ptr<Error> mError;

        META_EXPORT friend std::ostream &operator<<(std::ostream &out, const Result &result);
    };

    struct META_EXPORT ResultBuilder {
        Enum mType;
        const char *mFunction;
        const char *mFile;
        size_t mLine;
        std::ostringstream mMsg;

        ResultBuilder(Enum type, const char *function, const char *file, size_t line)
            : mType(type)
            , mFunction(function)
            , mFile(file)
            , mLine(line)
        {
        }

        operator Result();
        operator Error();

        template <typename T>
        ResultBuilder &&operator<<(T &&t) &&
        {
            mMsg << std::forward<T>(t);
            return std::move(*this);
        }
    };

#define REFLECT_ERROR(Type) \
    ::Engine::Reflect::ResultBuilder { Type, __func__, __FILE__, __LINE__ }

#define REFLECT_UNKNOWN_ERROR() REFLECT_ERROR(::Engine::GenericResult { ::Engine::GenericResult::UNKNOWN_ERROR })

#define REFLECT_PROPAGATE_ERROR(...)                     \
    if (::Engine::Reflect::Result _result = (__VA_ARGS__)) \
    return (_result.mError->mStackTrace.emplace_back(__func__, __FILE__, __LINE__), _result)

}
}