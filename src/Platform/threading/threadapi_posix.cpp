#include "../platformlib.h"

#if POSIX

#    if LINUX || ANDROID
#        include <sys/prctl.h>
#    endif

#    include <pthread.h>

#    include "threadapi.h"

namespace Engine {
namespace Platform {
    namespace Threading {

        void setCurrentThreadName(const std::string &name)
        {
#    if LINUX || ANDROID
            prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
#    elif EMSCRIPTEN
#    elif OSX || IOS
            pthread_setname_np(name.c_str());
#    elif WINDOWS
            pthread_setname_np(pthread_self(), name.c_str());
#    else
#        error "Unsupported Platform!"
#    endif
        }

        std::string getCurrentThreadName()
        {
#    if LINUX || ANDROID
            char buffer[17];
            prctl(PR_GET_NAME, buffer, 0, 0, 0);
            return buffer;
#    elif EMSCRIPTEN
            throw 0;
#    elif OSX || IOS || WINDOWS
            char buffer[512];
            pthread_getname_np(pthread_self(), buffer, 512);
            return buffer;
#    else
#        error "Unsupported Platform!"
#    endif
        }

    }
}
}

#endif
