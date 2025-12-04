#pragma once

/// @cond

#include "Madgine/behaviorlib.h"
#include "Madgine/codegen/codegenlib.h"
#include "Madgine/debuglib.h"
#include "Madgine/resourceslib.h"
#include "Modules/moduleslib.h"

#if defined(NodeGraph_EXPORTS)
#    define MADGINE_NODEGRAPH_EXPORT DLL_EXPORT
#else
#    define MADGINE_NODEGRAPH_EXPORT DLL_IMPORT
#endif

#include "nodegraphforward.h"

/// @endcond
