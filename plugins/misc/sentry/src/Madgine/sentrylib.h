#pragma once

#include "Madgine/rootlib.h"

#if defined(Sentry_EXPORTS)
#    define MADGINE_SENTRY_EXPORT DLL_EXPORT
#else
#    define MADGINE_SENTRY_EXPORT DLL_IMPORT
#endif

#include "sentryforward.h"