#pragma once

#include "Madgine_Tools/rendertoolslib.h"
#include "OpenGL/opengllib.h"

#if defined(OpenGLTools_EXPORTS)
#    define MADGINE_OPENGL_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_OPENGL_TOOLS_EXPORT DLL_IMPORT
#endif

#include "opengltoolsforward.h"