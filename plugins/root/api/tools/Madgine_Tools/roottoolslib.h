#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/rootlib.h"

#if defined(RootTools_EXPORTS)
#    define MADGINE_ROOT_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_ROOT_TOOLS_EXPORT DLL_IMPORT
#endif

#include "roottoolsforward.h"