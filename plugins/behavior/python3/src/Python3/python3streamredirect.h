#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct Python3StreamRedirect {

            Python3StreamRedirect(Log::Log *log = nullptr);
            Python3StreamRedirect(const Python3StreamRedirect &) = delete;
            ~Python3StreamRedirect();

            void redirect(std::string_view name);
            void reset(std::string_view name);

            int write(std::string_view text);

            void setLog(Log::Log *log);
            Log::Log *log();

        private:
            Log::Log *mLog;
            std::map<std::string_view, PyObject *> mOldStreams;
        };

    }
}
}