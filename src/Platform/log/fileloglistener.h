#pragma once

#include "loglistener.h"

namespace Engine {
namespace Platform {
    namespace Log {

        struct PLATFORM_EXPORT FileLogListener : LogListener {

            FileLogListener(const std::string &fileName);

            virtual void messageLogged(EMSCRIPTEN_WORKAROUND(std::string_view) message, MessageType lml, const char *file, size_t line, Log *log) override;

        private:
            std::ofstream mFile;
        };

    }
}
}