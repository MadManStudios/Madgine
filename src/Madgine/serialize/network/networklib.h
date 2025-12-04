#pragma once

#include "Interfaces/interfaceslib.h"
#include "Meta/metalib.h"

#if defined(NetworkSerialize_EXPORTS)
#    define MADGINE_NETWORK_SERIALIZE_EXPORT DLL_EXPORT
#else
#    define MADGINE_NETWORK_SERIALIZE_EXPORT DLL_IMPORT
#endif