#pragma once

#include "Madgine/codegen/codegenlib.h"
#include "Modules/moduleslib.h"

#if defined(Tools_EXPORTS)
#    define MADGINE_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_TOOLS_EXPORT DLL_IMPORT
#endif

#include <algorithm>

#include "toolsforward.h"
