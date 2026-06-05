#include "../platformlib.h"

#include "fileloglistener.h"

#include "loglistener.h"

namespace Engine {
namespace Platform {
    namespace Log {

        FileLogListener::FileLogListener(const std::string &fileName)
            : mFile(fileName)
        {
        }

        void FileLogListener::messageLogged(EMSCRIPTEN_WORKAROUND(std::string_view) message, MessageType lml, const char *file, size_t line, Log *log)
        {
            mFile << message << std::endl;
        }

    }
}
}