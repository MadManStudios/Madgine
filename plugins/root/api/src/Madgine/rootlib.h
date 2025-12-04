#pragma once

/// @cond

#include "Madgine/cli/clilib.h"
#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#if defined(Root_EXPORTS)
#    define MADGINE_ROOT_EXPORT DLL_EXPORT
#else
#    define MADGINE_ROOT_EXPORT DLL_IMPORT
#endif

#include <queue>
#include <set>

#include "rootforward.h"

/// @endcond
