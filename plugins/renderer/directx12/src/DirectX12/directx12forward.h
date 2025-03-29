#pragma once

namespace Engine {

namespace Render {
    struct DirectX12RenderWindow;

    struct DirectX12PersistentMeshData;
    struct DirectX12TempMeshData;

    struct DirectX12FontData;

    struct DirectX12RenderContext;
    struct DirectX12RenderTarget;

    struct DirectX12PersistentBuffer;
    struct DirectX12TempBuffer;

    struct DirectX12ConstantBufferHeap;

    struct DirectX12DescriptorHeap;
    struct DirectX12QueryHeap;

    struct DirectX12CommandAllocator;

    constexpr struct create_t {
    } create;
}

}
