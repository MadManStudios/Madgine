#pragma once

#include "Madgine/handlerlib.h"
#include "Madgine/widgetslib.h"

#if defined(WidgetHandler_EXPORTS)
#    define MADGINE_WIDGETHANDLER_EXPORT DLL_EXPORT
#else
#    define MADGINE_WIDGETHANDLER_EXPORT DLL_IMPORT
#endif
