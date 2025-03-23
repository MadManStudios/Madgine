#pragma once

/// @cond

#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#include "Madgine/debuglib.h"

#if defined(App_EXPORTS)
#    define MADGINE_APP_EXPORT DLL_EXPORT
#else
#    define MADGINE_APP_EXPORT DLL_IMPORT
#endif

#include "appforward.h"

#include <queue>
#include <set>

/// @endcond
