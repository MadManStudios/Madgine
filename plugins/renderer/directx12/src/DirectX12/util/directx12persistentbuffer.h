#pragma once

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12PersistentBuffer {

        DirectX12PersistentBuffer() = default;
        DirectX12PersistentBuffer(const ByteBuffer &data);
        DirectX12PersistentBuffer(const DirectX12PersistentBuffer &) = delete;
        DirectX12PersistentBuffer(DirectX12PersistentBuffer &&);
        ~DirectX12PersistentBuffer();

        DirectX12PersistentBuffer &operator=(const DirectX12PersistentBuffer &) = delete;
        DirectX12PersistentBuffer &operator=(DirectX12PersistentBuffer &&);

        explicit operator bool() const;

        void bindVertex(ID3D12GraphicsCommandList *commandList, UINT stride, size_t index = 0) const;
        void bindIndex(ID3D12GraphicsCommandList *commandList) const;

        void reset();
        void setData(const ByteBuffer &data);

    private:
        size_t mSize = 0;
        OffsetPtr mOffset;
    };

}
}