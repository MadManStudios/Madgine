#pragma once

#include "Generic/genericresult.h"

namespace Engine {

ENUM_BASE(BehaviorResult, GenericResult)

struct MADGINE_BEHAVIOR_EXPORT [[nodiscard]] BehaviorError {
    BehaviorError(GenericResult result = GenericResult::SUCCESS);
    BehaviorError(BehaviorResult result, const std::string &msg = "", const char *file = nullptr, size_t sourceLine = 0);

    MADGINE_BEHAVIOR_EXPORT friend std::ostream &operator<<(std::ostream &out, const BehaviorError &error);

    MADGINE_BEHAVIOR_EXPORT friend const char *tag_invoke(Log::get_file_name_t, BehaviorError &error);
    MADGINE_BEHAVIOR_EXPORT friend size_t tag_invoke(Log::get_line_nr_t, BehaviorError &error);

    BehaviorResult mResult;
    const char *mFile = nullptr;
    size_t mLineNumber = 0, mPosition = 0;
    std::string mMsg;
    std::string mNotes;
};

struct MADGINE_BEHAVIOR_EXPORT BehaviorErrorBuilder {
    BehaviorResult mType;
    const char *mFile;
    size_t mLine;
    std::ostringstream mMsg;

    BehaviorErrorBuilder(BehaviorResult type, const char *file, size_t line);

    operator BehaviorError();

    template <typename T>
    BehaviorErrorBuilder &&operator<<(T &&t) &&
    {
        mMsg << std::forward<T>(t);
        return std::move(*this);
    }
};

#define BEHAVIOR_ERROR(Type) \
    ::Engine::BehaviorErrorBuilder { Type, __FILE__, __LINE__ }
#define BEHAVIOR_UNKNOWN_ERROR() BEHAVIOR_ERROR(::Engine::BehaviorResult::UNKNOWN_ERROR)

}