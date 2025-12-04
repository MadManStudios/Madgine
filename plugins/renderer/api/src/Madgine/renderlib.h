#pragma once

/// @cond

#include "Madgine/codegen/codegenlib.h"
#include "Madgine/debuglib.h"
#include "Madgine/imageloaderlib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine/serialize/filesystem/filesystemlib.h"
#include "Madgine/serialize/memory/memorylib.h"
#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#if defined(Render_EXPORTS)
#    define MADGINE_RENDER_EXPORT DLL_EXPORT
#else
#    define MADGINE_RENDER_EXPORT DLL_IMPORT
#endif

#include "Madgine/render/renderforward.h"

#include "renderforward.h"

/// @endcond
