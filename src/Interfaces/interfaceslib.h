#pragma once

/// @cond

#include "Generic/genericlib.h"

#include "interfacesconfig.h"



#if defined(Interfaces_EXPORTS)
#    define INTERFACES_EXPORT DLL_EXPORT
#else
#    define INTERFACES_EXPORT DLL_IMPORT
#endif



#include "debug/memory/memoryglobal.h"


#include <array>
#include <fstream>
#include <optional>
#include <cstring>


#include "interfacesforward.h"
#include "log/logmethods.h"

/// @endcond
