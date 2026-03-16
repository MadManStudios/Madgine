#include "../metalib.h"

#include "keyvalueresult.h"

namespace Engine {

KeyValueResult::KeyValueResult(std::unique_ptr<KeyValueError> error)
    : mError(std::move(error))
{
}

KeyValueResult::KeyValueResult(const KeyValueResult &other)
    : mError(other.mError ? std::make_unique<KeyValueError>(*other.mError) : std::unique_ptr<KeyValueError> {})
{
}

KeyValueError::KeyValueError(GenericResult state, const std::string &msg)
    : mState(state)
    , mMsg(msg)
{
}

KeyValueError::KeyValueError(GenericResult state, const std::string &msg, const char *function, const char *file, size_t sourceLine)
    : KeyValueError(state, msg)
{
    mStackTrace.emplace_back(StackEntry { function, file, sourceLine });
}

std::ostream &operator<<(std::ostream &out, const KeyValueError &error)
{
    out << error.mState << '\n';
    out << error.mMsg;
    for (const KeyValueError::StackEntry &entry : error.mStackTrace) {
        out << "\n"
            << entry.mFunction << " (" << entry.mFile << ":" << entry.mLineNr << ")";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const KeyValueResult &result)
{
    if (result.mError)
        out << *result.mError;
    else
        out << "No Error";
    return out;
}

KeyValueResultBuilder::operator KeyValueResult()
{
    assert(mType != GenericResult::SUCCESS);
    return std::make_unique<KeyValueError>(mType, mMsg.str(), mFunction, mFile, mLine);
}

KeyValueResultBuilder::operator KeyValueError()
{
    assert(mType != GenericResult::SUCCESS);
    return {
        mType, mMsg.str(), mFunction, mFile, mLine
    };
}

KeyValueResult::operator bool() const
{
    return static_cast<bool>(mError);
}

}