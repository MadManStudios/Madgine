#pragma once

#include "Madgine/clientlib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine_Tools/templateslib.h"
#include "Madgine_Tools/toolslib.h"

#if defined(ClientTools_EXPORTS)
#    define MADGINE_CLIENT_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_CLIENT_TOOLS_EXPORT DLL_IMPORT
#endif

#include "clienttoolsforward.h"