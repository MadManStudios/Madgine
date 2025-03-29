#pragma once

#include "Generic/allocator/concepts.h"

#include "Madgine/render/future.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12Buffer {

        DirectX12Buffer() = default;
        DirectX12Buffer(const ByteBuffer &data, D3D12_RESOURCE_STATES targetState);
        DirectX12Buffer(const DirectX12Buffer &) = delete;
        DirectX12Buffer(DirectX12Buffer &&);
        ~DirectX12Buffer();

        DirectX12Buffer &operator=(const DirectX12Buffer &) = delete;
        DirectX12Buffer &operator=(DirectX12Buffer &&);

        explicit operator bool() const;

        void bindVertex(ID3D12GraphicsCommandList *commandList, UINT stride, size_t index = 0) const;
        void bindIndex(ID3D12GraphicsCommandList *commandList) const;

        void reset();
        RenderFuture setData(const ByteBuffer &data, D3D12_RESOURCE_STATES targetState);
        
        ID3D12Resource *resource() const;

        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress() const;

    private:
        Block mBlock;
    };

}
}