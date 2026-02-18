#include "../behaviorlib.h"

#include "behaviorerror.h"

namespace Engine {
namespace Behavior {

    BehaviorError::BehaviorError(GenericResult result)
        : mResult(result)
    {
    }

    BehaviorError::BehaviorError(BehaviorResult result, const std::string &msg, const char *file, size_t sourceLine)
        : mResult(result)
        , mFile(file)
        , mLineNumber(sourceLine)
        , mMsg(msg)
    {
    }

    BehaviorError::BehaviorError(const Execution::BindingError &error)
        : BehaviorError(BehaviorResult::UNKNOWN_ERROR, "Binding unavailable")
    {
    }

    std::ostream &operator<<(std::ostream &out, const BehaviorError &error)
    {
        return out << error.mResult << "\n"
                   << error.mMsg << "\n"
                   << error.mNotes;
    }

    const char *tag_invoke(Log::get_file_name_t, BehaviorError &error)
    {
        return error.mFile;
    }

    size_t tag_invoke(Log::get_line_nr_t, BehaviorError &error)
    {
        return error.mLineNumber;
    }

    BehaviorErrorBuilder::BehaviorErrorBuilder(BehaviorResult type, const char *file, size_t line)
        : mType(type)
        , mFile(file)
        , mLine(line)
    {
    }

    BehaviorErrorBuilder::operator BehaviorError()
    {
        return {
            mType, mMsg.str(), mFile, mLine
        };
    }

}
}