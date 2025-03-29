#pragma once

#include "Madgine/curl/curllib.h"
#include "Modules/moduleslib.h"
#include "Madgine/codegen/codegenlib.h"

#if defined(Tools_EXPORTS)
#    define MADGINE_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_TOOLS_EXPORT DLL_IMPORT
#endif

#include "toolsforward.h"

#include <algorithm>