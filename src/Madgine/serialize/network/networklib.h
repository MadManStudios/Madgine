#pragma once

#include "Meta/metalib.h"
#include "Interfaces/interfaceslib.h"

#if defined(NetworkSerialize_EXPORTS)
#    define MADGINE_NETWORK_SERIALIZE_EXPORT DLL_EXPORT
#else
#    define MADGINE_NETWORK_SERIALIZE_EXPORT DLL_IMPORT
#endif