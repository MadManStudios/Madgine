#pragma once

/// @cond

#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#include "Madgine/fontloaderlib.h"
#include "Madgine/pipelineloaderlib.h"
#include "Madgine/debuglib.h"

#if defined(Client_EXPORTS)
#    define MADGINE_CLIENT_EXPORT DLL_EXPORT
#else
#    define MADGINE_CLIENT_EXPORT DLL_IMPORT
#endif

#include "clientconfig.h"

#include "clientforward.h"

#include "Madgine/render/renderforward.h"


/// @endcond
