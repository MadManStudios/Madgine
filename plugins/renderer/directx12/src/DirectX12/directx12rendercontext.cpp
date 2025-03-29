#include "directx12lib.h"

#include "directx12renderwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "directx12rendercontext.h"
#include "directx12rendertexture.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Modules/threading/workgroupstorage.h"

#include "directx12meshloader.h"
#include "directx12pipelineloader.h"
#include "directx12textureloader.h"

#include "Madgine/render/constantvalues.h"

UNIQUECOMPONENT(Engine::Render::DirectX12RenderContext)

METATABLE_BEGIN(Engine::Render::DirectX12RenderContext)
METATABLE_END(Engine::Render::DirectX12RenderContext)

namespace Engine {
namespace Render {

    static void __stdcall dxDebugOutput(D3D12_MESSAGE_CATEGORY category,
        D3D12_MESSAGE_SEVERITY severity,
        D3D12_MESSAGE_ID id,
        LPCSTR message,
        void *context)
    {

        Log::MessageType lvl;
        switch (severity) {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            lvl = Log::MessageType::FATAL_TYPE;
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            lvl = Log::MessageType::ERROR_TYPE;
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            lvl = Log::MessageType::WARNING_TYPE;
            break;
        case D3D12_MESSAGE_SEVERITY_INFO:
            lvl = Log::MessageType::INFO_TYPE;
            break;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            lvl = Log::MessageType::DEBUG_TYPE;
            break;
        }

        Log::LogDummy cout(lvl);
        cout << "Debug message (" << id << "): " << message << "\n";
    }

    static ReleasePtr<IDXGIAdapter1> GetHardwareAdapter(IDXGIFactory4 *pFactory)
    {
        for (UINT adapterIndex = 0;; ++adapterIndex) {
            ReleasePtr<IDXGIAdapter1> pAdapter;
            if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
                // No more adapters to enumerate.
                break;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                return pAdapter;
            }
        }
        return {};
    }

    Threading::WorkgroupLocal<ReleasePtr<ID3D12Device>> sDevice;

    Threading::WorkgroupLocal<DirectX12RenderContext *> sSingleton = nullptr;

    ID3D12Device *GetDevice()
    {
        return *sDevice;
    }

    DirectX12RenderContext::DirectX12RenderContext(Threading::TaskQueue *queue)
        : Component(queue)
        , mGraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, "Graphics", &mDescriptorHeap, &mTimestampQueryHeap)
        , mCopyQueue(D3D12_COMMAND_LIST_TYPE_COPY, "Copy", &mDescriptorHeap, &mTimestampQueryHeap)
        , mComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE, "Compute", &mDescriptorHeap, &mTimestampQueryHeap)
        , mUploadAllocator(mUploadHeap)
        , mBufferMemoryHeap(mDescriptorHeap)
        , mBufferAllocator(mBufferMemoryHeap)
        , mTempAllocator(mTempMemoryHeap)
        , mReadbackAllocator(mReadbackMemoryHeap)
        , mConstantMemoryHeap(mDescriptorHeap)
        , mConstantAllocator(mConstantMemoryHeap)

    {

        assert(!sSingleton);

        sSingleton = this;

        HRESULT hr;

        /* {
            ReleasePtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                debugController->EnableDebugLayer();
                ReleasePtr<ID3D12Debug1> debugController1;
                if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1)))) {
                    debugController1->SetEnableGPUBasedValidation(true);
                    DX12_LOG("Enabled Debug Layer");
                }
            }
        }*/

        assert(*sDevice == nullptr);

        hr = CreateDXGIFactory1(IID_PPV_ARGS(&mFactory));
        DX12_CHECK(hr);

        ReleasePtr<IDXGIAdapter1> hardwareAdapter = GetHardwareAdapter(mFactory);

        DX12_LOG("Creating Device...");
        hr = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&*sDevice));
        DX12_CHECK(hr);
        DX12_LOG("Success");

        {
            ReleasePtr<ID3D12InfoQueue1> infoQueue;
            hr = GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue));

            if (SUCCEEDED(hr)) {
                hr = infoQueue->RegisterMessageCallback(
                    &dxDebugOutput,
                    D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                    nullptr,
                    &mCallbackCookie);
                DX12_CHECK(hr);
            }
        }

        hr = GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &mOptions, sizeof(mOptions));
        DX12_CHECK(hr);

        mDescriptorHeap = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        mRenderTargetDescriptorHeap = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        mDepthStencilDescriptorHeap = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        mTimestampQueryHeap = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

        mBufferMemoryHeap.setup(256);
        mConstantMemoryHeap.setup(256);

        createRootSignature();

        mGraphicsQueue.setup();
        mCopyQueue.setup();
        mComputeQueue.setup();

        ConstantValues values;
        mConstantBuffer.setData({ &values, sizeof(values) }, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    DirectX12RenderContext::~DirectX12RenderContext()
    {
        ReleasePtr<ID3D12InfoQueue1> infoQueue;
        HRESULT hr = GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue));

        if (SUCCEEDED(hr)) {
            hr = infoQueue->UnregisterMessageCallback(mCallbackCookie);
            DX12_CHECK(hr);
        }

        mConstantBuffer.reset();

        mBufferAllocator.deallocateAll();
        mTempAllocator.deallocateAll();
        mConstantAllocator.deallocateAll();
        mUploadAllocator.deallocateAll();

        mTempMemoryHeap.reset();
        mConstantMemoryHeap.reset();
        mBufferMemoryHeap.reset();

        (*sDevice).reset();

        assert(sSingleton == this);
        sSingleton = nullptr;
    }

    std::unique_ptr<RenderTarget> DirectX12RenderContext::createRenderTexture(const Vector2i &size, const RenderTextureConfig &config)
    {
        return std::make_unique<DirectX12RenderTexture>(this, size, config);
    }

    bool DirectX12RenderContext::beginFrame()
    {
        if (!mGraphicsQueue.isComplete(mFrameFences[1 - (mFrame % 2)]))
            return false;
        return RenderContext::beginFrame();
    }

    void DirectX12RenderContext::endFrame()
    {
        RenderContext::endFrame();

        mFrameFences[mFrame % 2] = mGraphicsQueue.currentFence();
    }

    DirectX12RenderContext &DirectX12RenderContext::getSingleton()
    {
        return *sSingleton;
    }

    void DirectX12RenderContext::createRootSignature()
    {
        CD3DX12_ROOT_PARAMETER rootParameters[9];

        rootParameters[0].InitAsConstantBufferView(0);
        rootParameters[1].InitAsConstantBufferView(1);
        rootParameters[2].InitAsConstantBufferView(2);

        CD3DX12_DESCRIPTOR_RANGE bindlessRange;
        bindlessRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 0, 0);
        rootParameters[3].InitAsDescriptorTable(1, &bindlessRange);

        rootParameters[4].InitAsShaderResourceView(0, 1);

        CD3DX12_DESCRIPTOR_RANGE ranges[4];
        for (size_t i = 0; i < 4; ++i) {
            ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, i + 2);
            rootParameters[5 + i].InitAsDescriptorTable(1, ranges + i);
        }

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        D3D12_STATIC_SAMPLER_DESC samplerDesc[2];
        ZeroMemory(samplerDesc, sizeof(samplerDesc));

        samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc[0].MipLODBias = 0.0f;
        samplerDesc[0].MaxAnisotropy = 1;
        samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        samplerDesc[0].MinLOD = 0;
        samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;

        samplerDesc[1] = samplerDesc[0];

        samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc[1].ShaderRegister = 1;

        rootSignatureDesc.Init(9, rootParameters, 2, samplerDesc, rootSignatureFlags);

        ReleasePtr<ID3DBlob> signature;
        ReleasePtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        DX12_CHECK(hr);
        hr = GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
        DX12_CHECK(hr);
    }

    void DirectX12RenderContext::setupRootSignature(ID3D12GraphicsCommandList *list)
    {
        list->SetGraphicsRootSignature(mRootSignature);
        list->SetGraphicsRootDescriptorTable(3, mBufferMemoryHeap.descriptorTable());
    }

    DirectX12CommandList DirectX12RenderContext::fetchCommandList(D3D12_COMMAND_LIST_TYPE type)
    {
        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            return mGraphicsQueue.fetchCommandList();
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return mCopyQueue.fetchCommandList();
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return mComputeQueue.fetchCommandList();
        default:
            throw 0;
        }
    }

    DirectX12CommandAllocator *DirectX12RenderContext::graphicsQueue()
    {
        return &mGraphicsQueue;
    }

    std::unique_ptr<RenderTarget> DirectX12RenderContext::createRenderWindow(Window::OSWindow *w, size_t samples)
    {
        checkThread();

        return std::make_unique<DirectX12RenderWindow>(this, w, samples);
    }

    Threading::Task<void> DirectX12RenderContext::unloadAllResources()
    {
        co_await RenderContext::unloadAllResources();

        for (std::pair<const std::string, DirectX12PipelineLoader::Resource> &res : DirectX12PipelineLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }

        for (std::pair<const std::string, DirectX12TextureLoader::Resource> &res : DirectX12TextureLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }

        for (std::pair<const std::string, DirectX12MeshLoader::Resource> &res : DirectX12MeshLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }

        mGraphicsQueue.waitForIdle();
        mComputeQueue.waitForIdle();
        mCopyQueue.waitForIdle();
    }

    bool DirectX12RenderContext::supportsMultisampling() const
    {
        return true;
    }

    GPUBuffer<void> DirectX12RenderContext::allocateBufferImpl(size_t size)
    {
        GPUBuffer<void> result;
        Block allocation = mBufferAllocator.allocate(size);

        if (!allocation.mAddress)
            return {};

        result.mPtr = GPUPtr<void> { allocation.mAddress };
        result.mSize = allocation.mSize;

        /* auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(result.mSize);
        GetDevice()->CreatePlacedResource(
            mDefaultMemoryHeap.heap(result.mPtr.mBuffer),
            result.mPtr.mOffset,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&result.mBuffer.setupAs<ID3D12Resource>()));*/
        result.mBuffer.setupAs<ID3D12Resource *>() = mBufferMemoryHeap.resolve(allocation.mAddress).first;
        return result;
    }

    void DirectX12RenderContext::deallocateBufferImpl(GPUBuffer<void> buffer)
    {
        buffer.mBuffer.release<ID3D12Resource *>();
        mBufferAllocator.deallocate({ buffer.mPtr.encode(), buffer.mSize });
    }

    WritableByteBuffer DirectX12RenderContext::mapBufferImpl(GPUBuffer<void> &buffer)
    {
        Block uploadAllocation = mUploadAllocator.allocate(buffer.mSize);

        struct Deleter {
            void operator()(void *ptr)
            {
                auto list = mContext->mGraphicsQueue.fetchCommandList();
                list.Transition(mResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
                auto [heap, offset] = mContext->mUploadHeap.resolve(ptr);
                list->CopyBufferRegion(mResource, mOffset, heap, offset, mSize);
                list.Transition(mResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

                struct Deleter {

                    void operator()(void *ptr)
                    {
                        mContext->mUploadAllocator.deallocate({ ptr, mSize });
                    }

                    DirectX12RenderContext *mContext;
                    size_t mSize;
                };
                list.attachResource(std::unique_ptr<void, Deleter> { ptr, { mContext, mSize } });
                list.execute();
            }

            size_t mOffset;
            ID3D12Resource *mResource;
            DirectX12RenderContext *mContext;
            size_t mSize;
        };

        return {
            std::unique_ptr<void, Deleter> { uploadAllocation.mAddress, { buffer.mPtr.mOffset, buffer.mBuffer.as<ID3D12Resource *>(), this, uploadAllocation.mSize } }, uploadAllocation.mSize
        };
    }

    UniqueResourceBlock DirectX12RenderContext::createResourceBlock(std::vector<const Texture *> textures)
    {
        OffsetPtr offset = mDescriptorHeap.allocate(textures.size());

        UniqueResourceBlock block;

        block.setupAs<D3D12_GPU_DESCRIPTOR_HANDLE>() = mDescriptorHeap.gpuHandle(offset);

        for (size_t i = 0; i < textures.size(); ++i) {
            static_cast<const DirectX12Texture *>(textures[i])->createShaderResourceView(offset + i);
        }

        return block;
    }

    void DirectX12RenderContext::destroyResourceBlock(UniqueResourceBlock &block)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = block.release<D3D12_GPU_DESCRIPTOR_HANDLE>();
        mDescriptorHeap.deallocate(handle);
    }

    static constexpr const char *vSemantics[] = {
        "POSITION",
        "POSITION",
        "POSITION",
        "NORMAL",
        "COLOR",
        "TEXCOORD",
        "BONEINDICES",
        "WEIGHTS"
    };

    static constexpr unsigned int vSemanticIndices[] = {
        0,
        1,
        2,
        0,
        0,
        0,
        0,
        0,
    };

    static constexpr DXGI_FORMAT vFormats[] = {
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_SINT,
        DXGI_FORMAT_R32G32B32A32_FLOAT
    };

    std::vector<D3D12_INPUT_ELEMENT_DESC> DirectX12RenderContext::createVertexLayout(VertexFormat format)
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayoutDesc;

#ifndef NDEBUG
#    define semantic(i) vSemantics[i]
#    define semanticIndex(i) vSemanticIndices[i]
#else
#    define semantic(i) "TEXCOORD"
#    define semanticIndex(i) (UINT) i
#endif

        UINT offset = 0;
        for (UINT i = 0; i < VertexElements::size; ++i) {
            if (format.has(i)) {
                vertexLayoutDesc.push_back({ semantic(i),
                    semanticIndex(i), vFormats[i], 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
                offset += sVertexElementSizes[i];
            } else {
                vertexLayoutDesc.push_back({ semantic(i),
                    semanticIndex(i), vFormats[i], 2, vConstantOffsets[i], D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            }
        }

        return vertexLayoutDesc;
    }

}
}
