#pragma once

#include "Madgine/render/pipelineinstance.h"

#include "../directx12pipelineloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12PipelineInstance : PipelineInstance {

        DirectX12PipelineInstance(const PipelineConfiguration &config, DirectX12PipelineLoader::Handle pipeline);

        bool bind(DirectX12RenderTarget *target, VertexFormat vertexFormat, size_t groupSize) const;
        void bindRootSignature(DirectX12RenderTarget *target) const;

        virtual Memory::WritableByteBuffer mapParameters(size_t index) override;
        virtual Memory::WritableByteBuffer mapTempBuffer(size_t space, size_t elementSize, size_t count) const override;

        virtual void bindMesh(RenderTarget *target, const GPUMeshData &mesh) const override;
        virtual Memory::WritableByteBuffer mapVertices(RenderTarget *target, VertexFormat format, size_t count) const override;
        virtual Memory::TypedByteBuffer<uint32_t> mapIndices(RenderTarget *target, size_t count) const override;
        virtual void setGroupSize(size_t groupSize) const override;

        virtual void render(RenderTarget *target) const override;
        virtual void renderRange(RenderTarget *target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset = {}) const override;
        virtual void renderInstanced(RenderTarget *target, size_t count) const override;

        virtual void bindResources(RenderTarget *target, size_t space, ResourceBlock block) const override;

    private:
        DirectX12PipelineLoader::Handle mPipeline;

        std::vector<size_t> mConstantBufferSizes;
        std::vector<D3D12_GPU_VIRTUAL_ADDRESS> mConstantGPUAddresses;

        bool mDepthChecking;
        bool mDepthWriting;

        mutable std::vector<D3D12_GPU_VIRTUAL_ADDRESS> mTempGPUAddresses;

        DirectX12PipelineLoader::Handle mPipelineHandle;

        mutable bool mHasIndices = false;
        mutable UINT mElementCount;
        mutable VertexFormat mFormat;
        mutable size_t mGroupSize;
    };

}
}