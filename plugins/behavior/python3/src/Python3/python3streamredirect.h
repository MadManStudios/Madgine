#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyObject *sPyLogContextVar;

        struct Python3StreamRedirect {

            Python3StreamRedirect();
            Python3StreamRedirect(const Python3StreamRedirect &) = delete;
            ~Python3StreamRedirect();

            void redirect(std::string_view name);
            void reset(std::string_view name);

            int write(std::string_view text);

        private:
            std::map<std::string_view, PyObject *> mOldStreams;
        };

    }
}
}