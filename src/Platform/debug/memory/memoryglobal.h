#pragma once

#if ENABLE_MEMTRACKING

#    if WINDOWS
#        define _CRTDBG_MAP_ALLOC
#        include <crtdbg.h>
#        include <stdlib.h>
#    endif

#endif