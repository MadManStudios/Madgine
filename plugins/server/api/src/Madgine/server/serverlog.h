#pragma once

#include "Interfaces/log/standardlog.h"

namespace Engine {
namespace Server {
    struct MADGINE_SERVER_EXPORT ServerLog : Log::StandardLog {
        ServerLog(const std::string &name);
        ~ServerLog();

        void log(std::string_view msg, Engine::Log::MessageType lvl, const char *file, size_t line) override;

        bool startConsole();
        void stopConsole();

        std::vector<std::string> update();

    private:
        bool mConsole;
        std::string mCurrentCmd;
    };
}
}
