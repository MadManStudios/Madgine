#pragma once

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendercontextcollector.h"

#include "util/directx12commandlist.h"
#include "util/directx12descriptorheap.h"

#include "Madgine/render/vertexformat.h"

#include "util/directx12buffer.h"

#include "util/directx12heapallocator.h"

#include "util/directx12commandallocator.h"

#include "util/directx12queryheap.h"

#include "Generic/allocator/bucket.h"
#include "Generic/allocator/heap.h"
#include "Generic/allocator/fixed.h"
#include "Generic/allocator/bump.h"

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

        virtual GPUBuffer<void> allocateBufferImpl(size_t size) override;
        virtual void deallocateBufferImpl(GPUBuffer<void> buffer) override;
        virtual WritableByteBuffer mapBufferImpl(GPUBuffer<void> &buffer) override;

        virtual UniqueResourceBlock createResourceBlock(std::vector<const Texture*> textures) override;
        virtual void destroyResourceBlock(UniqueResourceBlock &block) override;

        static DirectX12RenderContext &getSingleton();

        void createRootSignature();
        void setupRootSignature(ID3D12GraphicsCommandList *list);

        DirectX12CommandList fetchCommandList(D3D12_COMMAND_LIST_TYPE type);

        DirectX12CommandAllocator *graphicsQueue();

        static std::vector<D3D12_INPUT_ELEMENT_DESC> createVertexLayout(VertexFormat format);

        DirectX12DescriptorHeap mDescriptorHeap;
        DirectX12DescriptorHeap mRenderTargetDescriptorHeap;
        DirectX12DescriptorHeap mDepthStencilDescriptorHeap;
        
        ReleasePtr<ID3D12RootSignature> mRootSignature;
        ReleasePtr<IDXGIFactory4> mFactory;

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

        DirectX12Buffer mConstantBuffer;

        DWORD mCallbackCookie;

        RenderFuture mFrameFences[2];

        D3D12_FEATURE_DATA_D3D12_OPTIONS mOptions;
    };

}
}

REGISTER_TYPE(Engine::Render::DirectX12RenderContext)