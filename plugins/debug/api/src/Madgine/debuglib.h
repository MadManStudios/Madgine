#pragma once

#include "Madgine/rootlib.h"

#if defined(Debugger_EXPORTS)
#    define MADGINE_DEBUGGER_EXPORT DLL_EXPORT
#else
#    define MADGINE_DEBUGGER_EXPORT DLL_IMPORT
#endif

#include "debugforward.h"