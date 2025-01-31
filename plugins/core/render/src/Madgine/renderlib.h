#pragma once

/// @cond

#include "Meta/metalib.h"
#include "Modules/moduleslib.h"
#include "Madgine/codegen/codegenlib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine/fontloaderlib.h"
#include "Madgine/debuglib.h"

#if defined(Render_EXPORTS)
#    define MADGINE_RENDER_EXPORT DLL_EXPORT
#else
#    define MADGINE_RENDER_EXPORT DLL_IMPORT
#endif

#include "renderforward.h"

#include "Madgine/render/renderforward.h"


/// @endcond
