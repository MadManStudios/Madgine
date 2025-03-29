#include "../interfaceslib.h"

#include "log.h"
#include "standardlog.h"

#if WINDOWS
#    define NOMINMAX
#    include <Windows.h>
#endif

namespace Engine {
namespace Log {

    extern StandardLog sGlobalLog;

    Log *sLog = &sGlobalLog;

    void setLog(Log *log)
    {
        sLog = log;
    }

    void log(std::string_view msg, MessageType level, const char *file, size_t line, Log *log)
    {
#if WINDOWS
        if (level == MessageType::FATAL_TYPE || level == MessageType::DEBUG_TYPE) {
            OutputDebugStringA(msg.data());
            OutputDebugStringA("\n");
        }
#endif
        if (!log)
            log = sLog;
        log->log(msg, level, file, line);
    }

    LogDummy::LogDummy(MessageType lvl, const char *file, size_t line, Log *log)
        : mLvl(lvl)
        , mFile(file)
        , mLine(line)
        , mLog(log)
    {
    }

    LogDummy::~LogDummy()
    {
        log(mStream.str(), mLvl, mFile, mLine, mLog);
    }

}
}
