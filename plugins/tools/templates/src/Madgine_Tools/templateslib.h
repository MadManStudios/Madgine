#pragma once

#include "Madgine/resourceslib.h"
#include "Madgine_Tools/toolslib.h"
#include "TemplateEngine/templatelib.h"

#if defined(Templates_EXPORTS)
#    define MADGINE_TEMPLATES_EXPORT DLL_EXPORT
#else
#    define MADGINE_TEMPLATES_EXPORT DLL_IMPORT
#endif

#include "templatesforward.h"
