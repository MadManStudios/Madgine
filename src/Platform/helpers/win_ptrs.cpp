#include "../platformlib.h"

#if WINDOWS

#    include "win_ptrs.h"

#    define NOMINMAX
#    include <Windows.h>

#    include "win_ptrs.h"

namespace Engine {
namespace Platform {

    UniqueHandle::~UniqueHandle()
    {
        if (mHandle != INVALID_HANDLE_VALUE)
            CloseHandle(mHandle);
    }

}
}

#endif
