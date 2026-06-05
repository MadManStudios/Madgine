#pragma once

#include "Platform/log/standardlog.h"

namespace Engine {
namespace Core {
    struct MADGINE_SERVER_EXPORT ServerLog : Platform::Log::StandardLog {
        ServerLog(const std::string &name);
        ~ServerLog();

        void log(std::string_view msg, Platform::Log::MessageType lvl, const char *file, size_t line) override;

        bool startConsole();
        void stopConsole();

        std::vector<std::string> update();

    private:
        bool mConsole;
        std::string mCurrentCmd;
    };
}
}
