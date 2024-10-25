#pragma once

#include "Interfaces/interfaceslib.h"


#if defined(CLI_EXPORTS)
#    define CLI_EXPORT DLL_EXPORT
#else
#    define CLI_EXPORT DLL_IMPORT
#endif

#include "cliforward.h"