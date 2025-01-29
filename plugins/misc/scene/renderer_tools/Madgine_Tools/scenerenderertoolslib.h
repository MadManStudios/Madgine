#pragma once

#include "Madgine_Tools/clienttoolslib.h"
#include "Madgine/scenerendererlib.h"
#include "Madgine_Tools/debugtoolslib.h"
#include "Madgine_Tools/behaviortoolslib.h"

#include "Madgine/serialize/filesystem/filesystemlib.h"
#include "Madgine/serialize/memory/memorylib.h"

#include "Madgine/pipelineloaderlib.h"

#if defined(SceneRendererTools_EXPORTS)
#    define MADGINE_SCENE_RENDERER_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_SCENE_RENDERER_TOOLS_EXPORT DLL_IMPORT
#endif

#include "scenerenderertoolsforward.h"