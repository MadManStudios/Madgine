#pragma once

#include "log.h"

namespace Engine {
namespace Log {
    struct INTERFACES_EXPORT StandardLog : Log {
        StandardLog(const std::string &name);

        static void sLog(std::string_view msg, MessageType lvl, const std::string &name = "Default");

        static void setLogLevel(MessageType lvl);

        // Inherited via Log
        void log(std::string_view msg, MessageType lvl, const char *file, size_t line) override;
        std::string getName() override;

    private:
        std::string mName;
    };
}
}
