#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct Python3StreamRedirect {

            Python3StreamRedirect(Platform::Log::Log *log = nullptr);
            Python3StreamRedirect(const Python3StreamRedirect &) = delete;
            ~Python3StreamRedirect();

            void redirect(std::string_view name);
            void reset(std::string_view name);

            int write(std::string_view text);

            Platform::Log::Log *setLog(Platform::Log::Log *log);
            Platform::Log::Log *log();

        private:
            Platform::Log::Log *mLog;
            std::map<std::string_view, PyObject *> mOldStreams;
        };

    }
}
}