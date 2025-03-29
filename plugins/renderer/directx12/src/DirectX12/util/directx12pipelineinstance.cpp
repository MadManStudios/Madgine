#include "../directx12lib.h"

#include "directx12pipelineinstance.h"

#include "../directx12rendercontext.h"

#include "../directx12meshdata.h"
#include "../directx12rendertarget.h"

#include "Generic/align.h"

namespace Engine {
namespace Render {

    static constexpr D3D12_PRIMITIVE_TOPOLOGY sModes[] {
        D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    };

    DirectX12PipelineInstance::DirectX12PipelineInstance(const PipelineConfiguration &config, const DirectX12Pipeline *pipeline)
        : PipelineInstance(config)
        , mPipeline(pipeline)
        , mConstantBufferSizes(config.bufferSizes)
        , mDepthChecking(config.depthChecking)
    {
        mConstantGPUAddresses.resize(config.bufferSizes.size());
        for (size_t &size : mConstantBufferSizes)
            size = alignTo(size, 256);
    }

    bool DirectX12PipelineInstance::bind(DirectX12RenderTarget *target, VertexFormat vertexFormat, size_t groupSize) const
    {
        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        size_t samplesBits = sqrt(target->samples());
        assert(samplesBits * samplesBits == target->samples());

        ID3D12PipelineState *pipeline = mPipeline->get(vertexFormat, groupSize, target, mDepthChecking);
        if (!pipeline) {
            return false;
        }
        commandList->SetPipelineState(pipeline);

        assert(groupSize > 0 && groupSize <= 3);
        D3D12_PRIMITIVE_TOPOLOGY mode = sModes[groupSize - 1];
        commandList->IASetPrimitiveTopology(mode);

        for (size_t i = 0; i < std::min(size_t { 3 }, mConstantGPUAddresses.size()); ++i) {
            if (mConstantGPUAddresses[i])
                commandList->SetGraphicsRootConstantBufferView(i, mConstantGPUAddresses[i]);
        }
        for (size_t i = 0; i < mTempGPUAddresses.size(); ++i) {
            if (mTempGPUAddresses[i])
                commandList->SetGraphicsRootShaderResourceView(4 + i, mTempGPUAddresses[i]);
        }

        DX12_CHECK();

        return true;
    }

    WritableByteBuffer DirectX12PipelineInstance::mapParameters(size_t index)
    {
        Block block = DirectX12RenderContext::getSingleton().mTempAllocator.allocate(mConstantBufferSizes[index], 256);
        auto [res, offset] = DirectX12RenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);
        mConstantGPUAddresses[index] = res->GetGPUVirtualAddress() + offset;

        return { block.mAddress, block.mSize };
    }

    void DirectX12PipelineInstance::render(RenderTarget *_target) const
    {
        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);
        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        if (!bind(target, mFormat, mGroupSize))
            return;

        if (mHasIndices){
            commandList->DrawIndexedInstanced(mElementCount, 1, 0, 0, 0);
        } else {
            commandList->DrawInstanced(mElementCount, 1, 0, 0);
        }

        mHasIndices = false;
    }

    void DirectX12PipelineInstance::renderRange(RenderTarget *_target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset) const
    {
        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);
        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        if (!bind(target, mFormat, mGroupSize))
            return;

        assert(elementCount <= mElementCount);

        if (mHasIndices) {
            assert(indexOffset);            
            commandList->DrawIndexedInstanced(elementCount, 1, indexOffset, vertexOffset, 0);
        } else {
            commandList->DrawInstanced(elementCount, 1, vertexOffset, 0);
        }
    }

    void DirectX12PipelineInstance::renderInstanced(RenderTarget *_target, size_t count) const
    {
        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);
        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        if (!bind(target, mFormat, mGroupSize))
            return;

        if (mHasIndices) {
            commandList->DrawIndexedInstanced(mElementCount, count, 0, 0, 0);
        } else {
            commandList->DrawInstanced(mElementCount, count, 0, 0);
        }

        mHasIndices = false;
    }

    WritableByteBuffer DirectX12PipelineInstance::mapTempBuffer(size_t space, size_t size) const
    {
        assert(space >= 1);
        if (mTempGPUAddresses.size() <= space - 1)
            mTempGPUAddresses.resize(space);

        Block block = DirectX12RenderContext::getSingleton().mTempAllocator.allocate(alignTo(size, 256));
        auto [res, offset] = DirectX12RenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);
        mTempGPUAddresses[space - 1] = res->GetGPUVirtualAddress() + offset;

        return { block.mAddress, block.mSize };
    }

    void DirectX12PipelineInstance::bindMesh(RenderTarget *_target, const GPUMeshData *m) const
    {

        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);

        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        const DirectX12MeshData *mesh = static_cast<const DirectX12MeshData *>(m);

        mFormat = mesh->mFormat;
        mGroupSize = mesh->mGroupSize;

        mesh->mVertices.bindVertex(commandList, mesh->mVertexSize);

        DirectX12RenderContext::getSingleton().mConstantBuffer.bindVertex(commandList, 0, 2);

        
        if (mesh->mIndices) {
            mesh->mIndices.bindIndex(commandList);
            mHasIndices = true;
        } else {
            mHasIndices = false;
        }

        mElementCount = mesh->mElementCount;
    }

    WritableByteBuffer DirectX12PipelineInstance::mapVertices(RenderTarget *_target, VertexFormat format, size_t count) const
    {
        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);

        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        Block block = DirectX12RenderContext::getSingleton().mTempAllocator.allocate(alignTo(format.stride() * count, 256));
        auto [res, offset] = DirectX12RenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = res->GetGPUVirtualAddress() + offset;
        view.SizeInBytes = block.mSize;
        view.StrideInBytes = format.stride();
        commandList->IASetVertexBuffers(0, 1, &view);
        DX12_LOG("Bind Vertex Buffer -> " << res->GetGPUVirtualAddress() + offset);

        mElementCount = count;
        mFormat = format;

        return { block.mAddress, block.mSize };
    }

    ByteBufferImpl<uint32_t> DirectX12PipelineInstance::mapIndices(RenderTarget *_target, size_t count) const
    {
        DirectX12RenderTarget *target = static_cast<DirectX12RenderTarget *>(_target);

        ID3D12GraphicsCommandList *commandList = target->mCommandList;

        Block block = DirectX12RenderContext::getSingleton().mTempAllocator.allocate(alignTo(sizeof(uint32_t) * count, 256));
        auto [res, offset] = DirectX12RenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = res->GetGPUVirtualAddress() + offset;
        view.SizeInBytes = block.mSize;
        view.Format = DXGI_FORMAT_R32_UINT;
        commandList->IASetIndexBuffer(&view);
        DX12_LOG("Bind Index Buffer -> " << res->GetGPUVirtualAddress() + offset);

        mElementCount = count;
        mHasIndices = true;

        return { static_cast<uint32_t*>(block.mAddress), block.mSize };
    }

    void DirectX12PipelineInstance::setGroupSize(size_t groupSize) const
    {
        mGroupSize = groupSize;
    }

    void DirectX12PipelineInstance::bindResources(RenderTarget *target, size_t space, ResourceBlock block) const
    {
        assert(space > 1);
        if (block)
            static_cast<DirectX12RenderTarget *>(target)->mCommandList->SetGraphicsRootDescriptorTable(3 + space, { block.mPtr });
    }

    DirectX12PipelineInstanceHandle::DirectX12PipelineInstanceHandle(const PipelineConfiguration &config, DirectX12PipelineLoader::Handle pipeline)
        : DirectX12PipelineInstance(config, &*pipeline)
        , mPipelineHandle(std::move(pipeline))
    {
    }

    DirectX12PipelineInstancePtr::DirectX12PipelineInstancePtr(const PipelineConfiguration &config, DirectX12PipelineLoader::Ptr pipeline)
        : DirectX12PipelineInstance(config, &*pipeline)
        , mPipelinePtr(std::move(pipeline))
    {
    }

}
}