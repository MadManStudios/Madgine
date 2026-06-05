#pragma once

/// @cond

#include "Generic/genericlib.h"

#include "platformconfig.h"

#if defined(Platform_EXPORTS)
#    define PLATFORM_EXPORT DLL_EXPORT
#else
#    define PLATFORM_EXPORT DLL_IMPORT
#endif


#if EMSCRIPTEN
#    define EMSCRIPTEN_WORKAROUND(x) const x &
#else
#    define EMSCRIPTEN_WORKAROUND(x) x
#endif


#include <array>
#include <cstring>
#include <fstream>
#include <optional>

#include "debug/memory/memoryglobal.h"
#include "platformforward.h"
#include "log/logmethods.h"

/// @endcond
