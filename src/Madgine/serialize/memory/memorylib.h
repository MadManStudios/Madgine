#pragma once

#include "Meta/metalib.h"

#if defined(MemorySerialize_EXPORTS)
#    define MADGINE_MEMORY_SERIALIZE_EXPORT DLL_EXPORT
#else
#    define MADGINE_MEMORY_SERIALIZE_EXPORT DLL_IMPORT
#endif