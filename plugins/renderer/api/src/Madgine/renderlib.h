#pragma once

/// @cond

#include "Meta/metalib.h"
#include "Modules/moduleslib.h"
#include "Madgine/codegen/codegenlib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine/imageloaderlib.h"
#include "Madgine/debuglib.h"
#include "Madgine/serialize/memory/memorylib.h"
#include "Madgine/serialize/filesystem/filesystemlib.h"

#if defined(Render_EXPORTS)
#    define MADGINE_RENDER_EXPORT DLL_EXPORT
#else
#    define MADGINE_RENDER_EXPORT DLL_IMPORT
#endif

#include "renderforward.h"

#include "Madgine/render/renderforward.h"


/// @endcond
