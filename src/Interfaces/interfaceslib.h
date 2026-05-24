#pragma once

/// @cond

#include "Generic/genericlib.h"

#include "interfacesconfig.h"

#if defined(Interfaces_EXPORTS)
#    define INTERFACES_EXPORT DLL_EXPORT
#else
#    define INTERFACES_EXPORT DLL_IMPORT
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
#include "interfacesforward.h"
#include "log/logmethods.h"

/// @endcond
