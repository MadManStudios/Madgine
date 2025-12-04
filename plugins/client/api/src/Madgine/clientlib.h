#pragma once

/// @cond

#include "Madgine/debuglib.h"
#include "Madgine/renderlib.h"
#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#if defined(Client_EXPORTS)
#    define MADGINE_CLIENT_EXPORT DLL_EXPORT
#else
#    define MADGINE_CLIENT_EXPORT DLL_IMPORT
#endif

#include "Madgine/render/renderforward.h"

#include "clientconfig.h"
#include "clientforward.h"

/// @endcond
