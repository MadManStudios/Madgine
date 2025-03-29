#pragma once

/// @cond

#include "Modules/moduleslib.h"
#include "Meta/metalib.h"

#if defined(Server_EXPORTS)
#    define MADGINE_SERVER_EXPORT DLL_EXPORT
#else
#    define MADGINE_SERVER_EXPORT DLL_IMPORT
#endif

#include "serverforward.h"

#include <queue>
#include <set>

/// @endcond
