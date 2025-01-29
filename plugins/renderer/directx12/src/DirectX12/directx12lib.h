#pragma once

/// @cond

#include "Madgine/meshloaderlib.h"
#include "Madgine/pipelineloaderlib.h"
#include "Madgine/textureloaderlib.h"
#include "Madgine/renderlib.h"

#if defined(DirectX12_EXPORTS)
#    define MADGINE_DIRECTX12_EXPORT DLL_EXPORT
#else
#    define MADGINE_DIRECTX12_EXPORT DLL_IMPORT
#endif

#include "directx12forward.h"

#define NOMINMAX

#include <d3d12.h>
#include <dxgi1_6.h>
#include <pix.h>
#include "d3dx12.h"
#include <dxcapi.h>
#include <comdef.h>

MADGINE_DIRECTX12_EXPORT void dx12Dump(HRESULT result, const char *file, size_t line);
MADGINE_DIRECTX12_EXPORT bool checkDevice(HRESULT &result);

inline void dx12Check(const char *file, size_t line, HRESULT result = 0)
{
    if (FAILED(result) || !checkDevice(result)) {
        _com_error error { result };
        LOG_FATAL("DX12-Error (" << result << "): " << error.ErrorMessage());
        dx12Dump(result, file, line);
        std::terminate();
    }
}

#define DX12_CHECK(...) dx12Check(__FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

#define DX12_LOG(x) LOG_DEBUG("DX12: " << x)
//#define DX12_LOG(x)

#include "Generic/offsetptr.h"

#include "Interfaces/helpers/win_ptrs.h"
#include "Interfaces/helpers/win_wstring.h"

constexpr D3D12_CPU_DESCRIPTOR_HANDLE operator+(D3D12_CPU_DESCRIPTOR_HANDLE handle, Engine::OffsetPtr offset)
{
    return { handle.ptr + offset.offset() };
}

constexpr D3D12_GPU_DESCRIPTOR_HANDLE operator+(D3D12_GPU_DESCRIPTOR_HANDLE handle, Engine::OffsetPtr offset)
{
    return { handle.ptr + offset.offset() };
}

constexpr Engine::OffsetPtr operator-(D3D12_CPU_DESCRIPTOR_HANDLE handle1, D3D12_CPU_DESCRIPTOR_HANDLE handle2)
{
    return Engine::OffsetPtr { handle1.ptr - handle2.ptr };
}

constexpr Engine::OffsetPtr operator-(D3D12_GPU_DESCRIPTOR_HANDLE handle1, D3D12_GPU_DESCRIPTOR_HANDLE handle2)
{
    return Engine::OffsetPtr { handle1.ptr - handle2.ptr };
}

/// @endcond
