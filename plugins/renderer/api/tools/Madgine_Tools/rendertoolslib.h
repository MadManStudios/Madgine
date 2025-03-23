#pragma once

#include "Madgine_Tools/toolslib.h"
#include "Madgine/renderlib.h"
#include "Madgine/meshloaderlib.h"

#if defined(RenderTools_EXPORTS)
#    define MADGINE_RENDER_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_RENDER_TOOLS_EXPORT DLL_IMPORT
#endif

#include "rendertoolsforward.h"