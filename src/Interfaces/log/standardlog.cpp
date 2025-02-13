#include "../interfaceslib.h"
#include "standardlog.h"

#include <iostream>

#include <time.h>

#if ANDROID
#    include <android/log.h>
#endif

namespace Engine {
namespace Log {
    MessageType sLogLevel = MessageType::DEBUG_TYPE;
    extern Log *sLog;

    StandardLog sGlobalLog {"Default"};

    StandardLog::StandardLog(const std::string &name)
        : mName(name)
    {
        if (!Engine::Log::sLog) {
            Engine::Log::sLog = this;
        }
    }

    void StandardLog::sLog(std::string_view msg, MessageType lvl, const std::string &name)
    {
        if (lvl < sLogLevel)
            return;

#if ANDROID
        int prio;
        switch (lvl) {
        case MessageType::DEBUG_TYPE:
            prio = ANDROID_LOG_DEBUG;
            break;
        case MessageType::INFO_TYPE:
            prio = ANDROID_LOG_INFO;
            break;
        case MessageType::WARNING_TYPE:
            prio = ANDROID_LOG_WARN;
            break;
        case MessageType::ERROR_TYPE:
            prio = ANDROID_LOG_ERROR;
            break;
        case MessageType::FATAL_TYPE:
            prio = ANDROID_LOG_FATAL;
            break;
        default:
            std::terminate();
        }
        __android_log_print(prio, name.c_str(), "%s", msg.data());
#else

        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        struct tm *t;
#    if WINDOWS
        struct tm _t;
        t = &_t;
        localtime_s(t, &time);
#    else
        t = localtime(&time);
#    endif

        const char *type;
        switch (lvl) {
        case MessageType::DEBUG_TYPE:
            type = "Debug    :";
            break;
        case MessageType::INFO_TYPE:
            type = "Info     :";
            break;
        case MessageType::WARNING_TYPE:
            type = "Warning  :";
            break;
        case MessageType::ERROR_TYPE:
            type = "Error    :";
            break;
        case MessageType::FATAL_TYPE:
            type = "Fatal    :";
            break;
        }

        char s[30];
        strftime(s, 28, "%d/%m/%Y - %H:%M : ", t);
        std::cout << s << msg << std::endl;
#endif
    }

    void StandardLog::setLogLevel(MessageType lvl)
    {
        sLogLevel = lvl;
    }

    void StandardLog::log(std::string_view msg, MessageType lvl, const char *file, size_t line)
    {
        sLog(msg, lvl, mName);
        Log::log(msg, lvl, file, line);
    }

    std::string StandardLog::getName()
    {
        return mName;
    }

}
}
