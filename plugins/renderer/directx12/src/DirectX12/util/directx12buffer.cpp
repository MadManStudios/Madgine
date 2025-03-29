#include "../directx12lib.h"

#include "directx12buffer.h"

#include "../directx12rendercontext.h"

#include "Generic/bytebuffer.h"

#include "Generic/align.h"

namespace Engine {
namespace Render {

    DirectX12Buffer::DirectX12Buffer(DirectX12Buffer &&other)
        : mBlock(std::exchange(other.mBlock, {}))
    {
    }

    DirectX12Buffer::DirectX12Buffer(const ByteBuffer &data, D3D12_RESOURCE_STATES targetState)
    {
        setData(data, targetState);
    }

    DirectX12Buffer::~DirectX12Buffer()
    {
        reset();
    }

    DirectX12Buffer &DirectX12Buffer::operator=(DirectX12Buffer &&other)
    {
        std::swap(mBlock, other.mBlock);
        return *this;
    }

    DirectX12Buffer::operator bool() const
    {
        return mBlock.mAddress;
    }

    void DirectX12Buffer::bindVertex(ID3D12GraphicsCommandList *commandList, UINT stride, size_t index) const
    {
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = gpuAddress();
        view.SizeInBytes = mBlock.mSize;
        view.StrideInBytes = stride;
        commandList->IASetVertexBuffers(index, 1, &view);
        DX12_LOG("Bind Vertex Buffer -> " << gpuAddress());
    }

    void DirectX12Buffer::bindIndex(ID3D12GraphicsCommandList *commandList) const
    {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = gpuAddress();
        view.SizeInBytes = mBlock.mSize;
        view.Format = DXGI_FORMAT_R32_UINT;
        commandList->IASetIndexBuffer(&view);
        DX12_LOG("Bind Index Buffer -> " << gpuAddress());
    }

    void DirectX12Buffer::reset()
    {
        if (mBlock.mAddress) {
            DirectX12RenderContext::getSingleton().mConstantAllocator.deallocate(mBlock);
            mBlock = {};
        }
    }

    RenderFuture DirectX12Buffer::setData(const ByteBuffer &data, D3D12_RESOURCE_STATES targetState)
    {
        assert(data.mData);

        DirectX12RenderContext &context = DirectX12RenderContext::getSingleton();

        size_t expectedSize = alignTo(data.mSize, 256);
        if (mBlock.mSize != expectedSize) {
            reset();
            mBlock = context.mConstantAllocator.allocate(expectedSize);
        }

        Block uploadAllocation = context.mUploadAllocator.allocate(expectedSize);
        std::memcpy(uploadAllocation.mAddress, data.mData, data.mSize);

        auto [res, offset] = context.mConstantMemoryHeap.resolve(mBlock.mAddress);

        auto list = context.mGraphicsQueue.fetchCommandList();
        list.Transition(res, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        auto [heap, srcOffset] = context.mUploadHeap.resolve(uploadAllocation.mAddress);
        list->CopyBufferRegion(res, offset, heap, srcOffset, expectedSize);
        list.Transition(res, D3D12_RESOURCE_STATE_COPY_DEST, targetState);

        struct Deleter {

            void operator()(void *ptr)
            {
                DirectX12RenderContext::getSingleton().mUploadAllocator.deallocate({ ptr, mSize });
            }

            size_t mSize;
        };
        list.attachResource(std::unique_ptr<void, Deleter> { uploadAllocation.mAddress, { uploadAllocation.mSize } });
        return list.execute();
    }

    ID3D12Resource *DirectX12Buffer::resource() const
    {
        return DirectX12RenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress).first;
    }

    D3D12_GPU_VIRTUAL_ADDRESS DirectX12Buffer::gpuAddress() const
    {
        auto [res, offset] = DirectX12RenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress);
        return res->GetGPUVirtualAddress() + offset;
    }

}
}