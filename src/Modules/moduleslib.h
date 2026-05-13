#pragma once

#include "Interfaces/interfaceslib.h"

#include "modulesconfig.h"

/// @cond

#if defined(Modules_EXPORTS)
#    define MODULES_EXPORT DLL_EXPORT
#else
#    define MODULES_EXPORT DLL_IMPORT
#endif

#define MODULES_HAS_THREADS !EMSCRIPTEN

#include <regex>
#include <shared_mutex>
#include <stack>
#include <variant>

#include "modulesforward.h"

/// @endcond
