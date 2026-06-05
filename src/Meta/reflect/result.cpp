#include "../metalib.h"

#include "result.h"

namespace Engine {
namespace Reflect {

    Result::Result(std::unique_ptr<Error> error)
        : mError(std::move(error))
    {
    }

    Result::Result(const Result &other)
        : mError(other.mError ? std::make_unique<Error>(*other.mError) : std::unique_ptr<Error> {})
    {
    }

    Error::Error(Enum state, const std::string &msg)
        : mMsg(msg)
        , mState(state)
    {
    }

    Error::Error(Enum state, const std::string &msg, const char *function, const char *file, size_t sourceLine)
        : Error(state, msg)
    {
        mStackTrace.emplace_back(StackEntry { function, file, sourceLine });
    }

    Error::Error(Enum state, const std::string &msg, std::vector<StackEntry> stack)
        : mMsg(msg)
        , mStackTrace(std::move(stack))
        , mState(state)
    {
    }

    std::ostream &operator<<(std::ostream &out, const Error &error)
    {
        out << error.mState << '\n';
        out << error.mMsg;
        for (const Error::StackEntry &entry : error.mStackTrace) {
            out << "\n"
                << entry.mFunction << " (" << entry.mFile << ":" << entry.mLineNr << ")";
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, const Result &result)
    {
        if (result.mError)
            out << *result.mError;
        else
            out << "No Error";
        return out;
    }

    ResultBuilder::operator Result()
    {
        assert(mType != GenericResult { GenericResult::SUCCESS });
        return std::make_unique<Error>(mType, mMsg.str(), mFunction, mFile, mLine);
    }

    ResultBuilder::operator Error()
    {
        assert(mType != GenericResult { GenericResult::SUCCESS });
        return {
            mType, mMsg.str(), mFunction, mFile, mLine
        };
    }

    Result::operator bool() const
    {
        return static_cast<bool>(mError);
    }

}
}