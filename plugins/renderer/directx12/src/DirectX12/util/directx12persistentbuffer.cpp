#include "../directx12lib.h"

#include "directx12persistentbuffer.h"

#include "Generic/align.h"
#include "Generic/bytebuffer.h"

#include "../directx12rendercontext.h"

namespace Engine {
namespace Render {

    DirectX12PersistentBuffer::DirectX12PersistentBuffer(const ByteBuffer &data)
        : mSize(alignTo(data.mSize, 256))
    {
        if (mSize > 0) {
            DirectX12ConstantBufferHeap &heap = DirectX12RenderContext::getSingleton().mConstantBufferHeap;

            mOffset = heap.allocatePersistent(mSize);

            setData(data);
        }
    }

    DirectX12PersistentBuffer::DirectX12PersistentBuffer(DirectX12PersistentBuffer &&other)
        : mSize(std::exchange(other.mSize, 0))
        , mOffset(std::exchange(other.mOffset, {}))
    {
    }

    DirectX12PersistentBuffer::~DirectX12PersistentBuffer()
    {
        reset();
    }

    DirectX12PersistentBuffer &DirectX12PersistentBuffer::operator=(DirectX12PersistentBuffer &&other)
    {
        std::swap(mSize, other.mSize);
        std::swap(mOffset, other.mOffset);
        return *this;
    }

    DirectX12PersistentBuffer::operator bool() const
    {
        return mSize > 0;
    }

    void DirectX12PersistentBuffer::bindVertex(ID3D12GraphicsCommandList *commandList, UINT stride, size_t index) const
    {
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = DirectX12RenderContext::getSingleton().mConstantBufferHeap.addressPersistent(mOffset);
        view.SizeInBytes = mSize;
        view.StrideInBytes = stride;
        commandList->IASetVertexBuffers(index, 1, &view);
        DX12_LOG("Bind Vertex Buffer -> " << mBuffer);
    }

    void DirectX12PersistentBuffer::bindIndex(ID3D12GraphicsCommandList *commandList) const
    {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = DirectX12RenderContext::getSingleton().mConstantBufferHeap.addressPersistent(mOffset);
        view.SizeInBytes = mSize;
        view.Format = DXGI_FORMAT_R16_UINT;
        commandList->IASetIndexBuffer(&view);
        DX12_LOG("Bind Index Buffer -> " << mBuffer);
    }

    void DirectX12PersistentBuffer::reset()
    {
        if (mOffset) {
            DirectX12ConstantBufferHeap &heap = DirectX12RenderContext::getSingleton().mConstantBufferHeap;

            heap.deallocatePersistent(mOffset, mSize);
            mOffset.reset();
            mSize = 0;
        }
    }

    void DirectX12PersistentBuffer::setData(const ByteBuffer &data)
    {
        DirectX12ConstantBufferHeap &heap = DirectX12RenderContext::getSingleton().mConstantBufferHeap;
        if (mSize != alignTo(data.mSize, 256))
            *this = { data };
        else if (data.mData)
            heap.setData(mOffset, data);
    }

    /*
    D3D12_GPU_VIRTUAL_ADDRESS DirectX12Buffer::gpuAddress() const
    {
        DirectX12ConstantBufferHeap &heap = DirectX12RenderContext::getSingleton().mConstantBufferHeap;
        return mPersistent ? heap.addressPersistent(mOffset) : heap.addressTemp(mOffset);
    }*/

}
}