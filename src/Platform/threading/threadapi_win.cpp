#include "../platformlib.h"

#if WINDOWS && !POSIX

#    define NOMINMAX
#    include <Windows.h>

#    include "threadapi.h"

namespace Engine {
namespace Platform {
    namespace Threading {

        void setCurrentThreadName(const std::string &name)
        {
            std::wstring wstring;
            wstring.reserve(name.size());
            std::ranges::copy(name, std::back_inserter(wstring));
            SetThreadDescription(
                GetCurrentThread(),
                wstring.c_str());
        }

        std::string getCurrentThreadName()
        {
            std::string result;
            PWSTR buffer;
            if (SUCCEEDED(GetThreadDescription(GetCurrentThread(), &buffer))) {
                size_t len = wcslen(buffer);
                result.reserve(len);
                std::copy(buffer, buffer + len, std::back_inserter(result));
                LocalFree(buffer);
            }
            return result;
        }

    }
}
}

#endif
