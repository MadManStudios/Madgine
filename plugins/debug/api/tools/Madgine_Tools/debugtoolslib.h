#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/debuglib.h"
#include "Madgine/trees/treeslib.h"

#if defined(DebuggerTools_EXPORTS)
#    define MADGINE_DEBUGGER_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_DEBUGGER_TOOLS_EXPORT DLL_IMPORT
#endif

#include "debugtoolsforward.h"