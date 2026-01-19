#pragma once

#include "Generic/allocator/bucket.h"
#include "Generic/allocator/bump.h"
#include "Generic/allocator/fixed.h"
#include "Generic/allocator/heap.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendercontextcollector.h"
#include "Madgine/render/vertexformat.h"

#include "util/directx12commandallocator.h"
#include "util/directx12commandlist.h"
#include "util/directx12descriptorheap.h"
#include "util/directx12heapallocator.h"
#include "util/directx12queryheap.h"

namespace Engine {
namespace Render {

    MADGINE_DIRECTX12_EXPORT ID3D12Device *GetDevice();

    struct MADGINE_DIRECTX12_EXPORT DirectX12RenderContext : public RenderContextComponent<DirectX12RenderContext> {
        DirectX12RenderContext(Threading::TaskQueue *queue);
        ~DirectX12RenderContext();

        virtual std::unique_ptr<RenderTarget> createRenderWindow(Window::OSWindow *w, size_t samples) override;
        virtual std::unique_ptr<RenderTarget> createRenderTexture(const Vector2i &size = { 1, 1 }, const RenderTextureConfig &config = {}) override;

        virtual bool beginFrame() override;
        virtual void endFrame() override;

        virtual Threading::Task<void> unloadAllResources() override;

        virtual bool supportsMultisampling() const override;

        virtual GPUPtr<void> allocateBufferImpl(size_t size, UsageHint hint = UsageHint::USAGE_DEFAULT) override;    
        virtual GPUPtr<Void[]> allocateBufferImpl(size_t elementSize, size_t count, UsageHint hint = UsageHint::USAGE_DEFAULT) override;
        virtual WritableByteBuffer mapBufferImpl(const GPUPtr<void> &buffer) override;
        virtual WritableByteBuffer mapBufferImpl(const GPUPtr<Void[]> &buffer) override;

        virtual UniqueResourceBlock createResourceBlock(std::vector<std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>>> textures) override;
        virtual void destroyResourceBlock(UniqueResourceBlock &block) override;

        TexturePtr createTexture(TextureType type, TextureFormat format, Vector2i size, const ByteBuffer &data) override;

        void setTextureSubData(const TexturePtr &tex, Vector2i offset, Vector2i size, const ByteBuffer &data) override;

        static DirectX12RenderContext &getSingleton();
        
        ID3D12RootSignature *getRootSignature(const PipelineSignature &signature);
        void setupRootSignature(ID3D12RootSignature *signature, ID3D12GraphicsCommandList *list);

        DirectX12CommandList fetchCommandList(D3D12_COMMAND_LIST_TYPE type);

        DirectX12CommandAllocator *graphicsQueue();

        static std::vector<D3D12_INPUT_ELEMENT_DESC> createVertexLayout(VertexFormat format);

        DirectX12DescriptorHeap mDescriptorHeap;
        DirectX12DescriptorHeap mRenderTargetDescriptorHeap;
        DirectX12DescriptorHeap mDepthStencilDescriptorHeap;

        ReleasePtr<IDXGIFactory4> mFactory;

        std::map<PipelineSignature, ReleasePtr<ID3D12RootSignature>> mRootSignatures;

        DirectX12QueryHeap mTimestampQueryHeap;

        DirectX12CommandAllocator mGraphicsQueue;
        DirectX12CommandAllocator mCopyQueue;
        DirectX12CommandAllocator mComputeQueue;

        DirectX12MappedHeapAllocator mUploadHeap = D3D12_HEAP_TYPE_UPLOAD;
        BucketAllocator<HeapAllocator<DirectX12MappedHeapAllocator &>, 16, 64, 16> mUploadAllocator;

        DirectX12HeapAllocator mBufferMemoryHeap;
        LogBucketAllocator<HeapAllocator<DirectX12HeapAllocator &>, 64, 4096, 4> mBufferAllocator;

        DirectX12MappedHeapAllocator mTempMemoryHeap = D3D12_HEAP_TYPE_UPLOAD;
        BumpAllocator<FixedAllocator<DirectX12MappedHeapAllocator &>> mTempAllocator;

        DirectX12MappedHeapAllocator mReadbackMemoryHeap = D3D12_HEAP_TYPE_READBACK;
        BumpAllocator<FixedAllocator<DirectX12MappedHeapAllocator &>> mReadbackAllocator;

        DirectX12HeapAllocator mConstantMemoryHeap;
        LogBucketAllocator<HeapAllocator<DirectX12HeapAllocator &>, 64, 4096, 4> mConstantAllocator;

        GPUPtr<ConstantValues> mConstantBuffer;

        DWORD mCallbackCookie;

        RenderFuture mFrameFences[2];

        D3D12_FEATURE_DATA_D3D12_OPTIONS mOptions;
    };

}
}