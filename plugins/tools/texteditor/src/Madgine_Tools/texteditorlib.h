#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/resourceslib.h"

#if defined(TextEditor_EXPORTS)
#    define MADGINE_TEXTEDITOR_EXPORT DLL_EXPORT
#else
#    define MADGINE_TEXTEDITOR_EXPORT DLL_IMPORT
#endif

#include "texteditorforward.h"
