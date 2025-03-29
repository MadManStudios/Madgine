#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine_Tools/clienttoolslib.h"

#if defined(MadgineLauncherTools_EXPORTS)
#    define MADGINE_LAUNCHER_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_LAUNCHER_TOOLS_EXPORT DLL_IMPORT
#endif
