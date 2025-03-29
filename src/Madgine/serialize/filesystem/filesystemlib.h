#pragma once

#include "Meta/metalib.h"
#include "Interfaces/interfaceslib.h"

#if defined(FilesystemSerialize_EXPORTS)
#    define MADGINE_FILESYSTEM_SERIALIZE_EXPORT DLL_EXPORT
#else
#    define MADGINE_FILESYSTEM_SERIALIZE_EXPORT DLL_IMPORT
#endif