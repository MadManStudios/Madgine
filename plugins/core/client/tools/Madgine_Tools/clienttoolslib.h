#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/clientlib.h"
#include "Madgine/textureloaderlib.h"
#include "Madgine/meshloaderlib.h"

#if defined(ClientTools_EXPORTS)
#    define MADGINE_CLIENT_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_CLIENT_TOOLS_EXPORT DLL_IMPORT
#endif

#include "clienttoolsforward.h"