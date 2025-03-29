#include "../interfaceslib.h"

#if WINDOWS

#    include "win_ptrs.h"

#    define NOMINMAX
#    include <Windows.h>
#include "win_ptrs.h"

UniqueHandle::~UniqueHandle()
{
    if (mHandle != INVALID_HANDLE_VALUE)
        CloseHandle(mHandle);
}


#endif

