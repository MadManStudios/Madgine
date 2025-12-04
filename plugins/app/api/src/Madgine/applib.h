#pragma once

/// @cond

#include "Madgine/debuglib.h"
#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#if defined(App_EXPORTS)
#    define MADGINE_APP_EXPORT DLL_EXPORT
#else
#    define MADGINE_APP_EXPORT DLL_IMPORT
#endif

#include <queue>
#include <set>

#include "appforward.h"

/// @endcond
