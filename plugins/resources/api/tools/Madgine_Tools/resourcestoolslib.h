#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/resourceslib.h"

#if defined(ResourcesTools_EXPORTS)
#    define MADGINE_RESOURCES_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_RESOURCES_TOOLS_EXPORT DLL_IMPORT
#endif

#include "resourcestoolsforward.h"