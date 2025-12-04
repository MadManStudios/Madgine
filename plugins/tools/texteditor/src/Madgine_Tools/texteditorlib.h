#pragma once

#include "Madgine/resourceslib.h"
#include "Madgine_Tools/toolslib.h"

#if defined(TextEditor_EXPORTS)
#    define MADGINE_TEXTEDITOR_EXPORT DLL_EXPORT
#else
#    define MADGINE_TEXTEDITOR_EXPORT DLL_IMPORT
#endif

#include "texteditorforward.h"
