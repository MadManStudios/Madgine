#pragma once

#include "Interfaces/interfaceslib.h"
#include "Meta/metalib.h"

#if defined(FilesystemSerialize_EXPORTS)
#    define MADGINE_FILESYSTEM_SERIALIZE_EXPORT DLL_EXPORT
#else
#    define MADGINE_FILESYSTEM_SERIALIZE_EXPORT DLL_IMPORT
#endif