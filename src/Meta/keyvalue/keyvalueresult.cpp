#include "../metalib.h"

#include "keyvalueresult.h"

namespace Engine {

KeyValueResult::KeyValueResult(GenericResult state, std::unique_ptr<KeyValueError> error)
    : mState(state)
    , mError(std::move(error))
{
}

KeyValueResult::KeyValueResult(const KeyValueResult &other)
    : mState(other.mState)
    , mError(other.mError ? std::make_unique<KeyValueError>(*other.mError) : std::unique_ptr<KeyValueError> {})
{
}

KeyValueError::KeyValueError(const std::string &msg)
    : mMsg(msg)
{
}

KeyValueError::KeyValueError(const std::string &msg, const char *function, const char *file, size_t sourceLine)
    : KeyValueError(msg)
{
    mStackTrace.emplace_back(StackEntry { function, file, sourceLine });
}

std::ostream &operator<<(std::ostream &out, const KeyValueError &error)
{
    out << error.mMsg;
    for (const KeyValueError::StackEntry &entry : error.mStackTrace) {
        out << "\n"
            << entry.mFunction << " (" << entry.mFile << ":" << entry.mLineNr << ")";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const KeyValueResult &result)
{
    out << result.mState;
    if (result.mState != GenericResult::SUCCESS)
        out << '\n'
            << *result.mError;
    return out;
}

KeyValueResultBuilder::operator KeyValueResult()
{
    assert(mType != GenericResult::SUCCESS);
    return {
        mType,
        std::make_unique<KeyValueError>(mMsg.str(), mFunction, mFile, mLine)
    };
}

KeyValueResultBuilder::operator KeyValueError()
{
    assert(mType != GenericResult::SUCCESS);
    return {
        mMsg.str(), mFunction, mFile, mLine
    };
}

}