#pragma once

#include "Madgine/resourceslib.h"

#if defined(ImageLoader_EXPORTS)
#    define MADGINE_IMAGELOADER_EXPORT DLL_EXPORT
#else
#    define MADGINE_IMAGELOADER_EXPORT DLL_IMPORT
#endif

#include "imageloaderforward.h"