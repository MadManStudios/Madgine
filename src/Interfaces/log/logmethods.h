#pragma once

#include "messagetype.h"

#include "Generic/tag_invoke.h"

namespace Engine {
namespace Log {

    INTERFACES_EXPORT void setLog(Log *log = nullptr);
    INTERFACES_EXPORT void log(std::string_view msg, MessageType lvl, const char *file = nullptr, size_t line = 0, Log *log = nullptr);

    struct INTERFACES_EXPORT LogDummy {
        LogDummy(MessageType lvl, const char *file = nullptr, size_t line = 0, Log *log = nullptr);
        ~LogDummy();

        template <typename T>
        LogDummy &operator<<(T &&t)
        {
            mStream << std::forward<T>(t);
            return *this;
        }

    private:
        std::ostringstream mStream;
        MessageType mLvl;
        const char *mFile;
        size_t mLine;
        Log *mLog;
    };

#if ENABLE_DEBUG_LOGGING
#    define LOG_DEBUG(s) Engine::Log::LogDummy { Engine::Log::MessageType::DEBUG_TYPE } << s
#else
#    define LOG_DEBUG(s)
#endif
#define LOG(s) Engine::Log::LogDummy { Engine::Log::MessageType::INFO_TYPE, __FILE__, __LINE__ } << s
#define LOG_WARNING(s) Engine::Log::LogDummy { Engine::Log::MessageType::WARNING_TYPE, __FILE__, __LINE__ } << s
#define LOG_ERROR(s) Engine::Log::LogDummy { Engine::Log::MessageType::ERROR_TYPE, __FILE__, __LINE__ } << s
#define LOG_FATAL(s) Engine::Log::LogDummy { Engine::Log::MessageType::FATAL_TYPE, __FILE__, __LINE__ } << s

#define ONCE(f) std::call_once([]() -> std::once_flag & {static std::once_flag dummy; return dummy; }(), [&]() { f; })

#define LOG_ONCE(s) ONCE(LOG(s))
#define LOG_WARNING_ONCE(s) ONCE(LOG_WARNING(s))
#define LOG_ERROR_ONCE(s) ONCE(LOG_ERROR(s))

    struct get_file_name_t {
        template <typename T>
        friend auto tag_invoke(get_file_name_t, T &t)
        {
            return nullptr;
        }

        template <typename T>
        requires tag_invocable<get_file_name_t, T &>
        auto operator()(T &value) const
            noexcept(is_nothrow_tag_invocable_v<get_file_name_t, T &>)
                -> tag_invoke_result_t<get_file_name_t, T &>
        {
            return tag_invoke(*this, value);
        }
    };

    inline constexpr get_file_name_t get_file_name;

    struct get_line_nr_t {
        template <typename T>
        friend size_t tag_invoke(get_line_nr_t, T &t)
        {
            return 0;
        }

        template <typename T>
        requires tag_invocable<get_line_nr_t, T &>
        auto operator()(T &value) const
            noexcept(is_nothrow_tag_invocable_v<get_line_nr_t, T &>)
                -> tag_invoke_result_t<get_line_nr_t, T &>
        {
            return tag_invoke(*this, value);
        }
    };

    inline constexpr get_line_nr_t get_line_nr;

    struct get_log_t {

        using signature = Log*();

        template <typename T>
            requires(!tag_invocable<get_log_t, T &>)
        auto operator()(T &t) const
        {
            return nullptr;
        }

        template <typename T>
            requires tag_invocable<get_log_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_log_t, T &>)
                -> tag_invoke_result_t<get_log_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_log_t get_log;

}
}
