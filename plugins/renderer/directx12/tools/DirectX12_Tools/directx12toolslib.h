#pragma once

#include "DirectX12/directx12lib.h"
#include "Madgine_Tools/rendertoolslib.h"

#if defined(DirectX12Tools_EXPORTS)
#    define MADGINE_DIRECTX12_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_DIRECTX12_TOOLS_EXPORT DLL_IMPORT
#endif

#include "directx12toolsforward.h"