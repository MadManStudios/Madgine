#pragma once

/// @cond

#include "Madgine/codegen/codegenlib.h"
#include "Madgine/debuglib.h"
#include "Madgine/behaviorlib.h"

#include "Modules/moduleslib.h"
#include "Madgine/resourceslib.h"


#if defined(NodeGraph_EXPORTS)
#    define MADGINE_NODEGRAPH_EXPORT DLL_EXPORT
#else
#    define MADGINE_NODEGRAPH_EXPORT DLL_IMPORT
#endif

#include "nodegraphforward.h"

/// @endcond
