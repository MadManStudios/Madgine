#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine_Tools/texteditorlib.h"
#include "Madgine_Tools/debugtoolslib.h"
#include "Python3/python3lib.h"

#if defined(Python3Tools_EXPORTS)
#    define MADGINE_PYTHON3_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_PYTHON3_TOOLS_EXPORT DLL_IMPORT
#endif

#include "python3toolsforward.h"