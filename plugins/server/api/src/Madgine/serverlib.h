#pragma once

/// @cond

#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#if defined(Server_EXPORTS)
#    define MADGINE_SERVER_EXPORT DLL_EXPORT
#else
#    define MADGINE_SERVER_EXPORT DLL_IMPORT
#endif

#include <queue>
#include <set>

#include "serverforward.h"

/// @endcond
