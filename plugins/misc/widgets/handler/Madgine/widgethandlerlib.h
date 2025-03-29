#pragma once

#include "Madgine/widgetslib.h"
#include "Madgine/handlerlib.h"

#if defined(WidgetHandler_EXPORTS)
#    define MADGINE_WIDGETHANDLER_EXPORT DLL_EXPORT
#else
#    define MADGINE_WIDGETHANDLER_EXPORT DLL_IMPORT
#endif
