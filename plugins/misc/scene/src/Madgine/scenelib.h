#pragma once

/// @cond

#include "Madgine/animationloaderlib.h"
#include "Madgine/applib.h"
#include "Madgine/behaviorlib.h"
#include "Madgine/meshloaderlib.h"
#include "Madgine/skeletonloaderlib.h"

#if defined(Scene_EXPORTS)
#    define MADGINE_SCENE_EXPORT DLL_EXPORT
#else
#    define MADGINE_SCENE_EXPORT DLL_IMPORT
#endif

#include <queue>
#include <set>

#include "sceneforward.h"

/// @endcond
