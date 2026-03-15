#include "directx12lib.h"

#include "directx12rendercontext.h"

#include "Modules/threading/workgroupstorage.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/render/constantvalues.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "directx12pipelineloader.h"
#include "directx12rendertexture.h"
#include "directx12renderwindow.h"

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

        {
            ReleasePtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                debugController->EnableDebugLayer();
                ReleasePtr<ID3D12Debug1> debugController1;
                if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1)))) {
                    debugController1->SetEnableGPUBasedValidation(true);
                    DX12_LOG("Enabled Debug Layer");
                }
            }
        }

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

        mGraphicsQueue.setup();
        mCopyQueue.setup();
        mComputeQueue.setup();

        mConstantBuffer = allocateBuffer<ConstantValues>();
        auto values = mapBuffer(mConstantBuffer);
        *values = {};
    }

    DirectX12RenderContext::~DirectX12RenderContext()
    {
        ReleasePtr<ID3D12InfoQueue1> infoQueue;
        HRESULT hr = GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue));

        if (SUCCEEDED(hr)) {
            hr = infoQueue->UnregisterMessageCallback(mCallbackCookie);
            DX12_CHECK(hr);
        }

        mConstantBuffer = {};

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

    ID3D12RootSignature *DirectX12RenderContext::getRootSignature(const PipelineSignature &signature)
    {
        auto [it, b] = mRootSignatures.try_emplace(signature);
        if (b) {
            CD3DX12_ROOT_PARAMETER rootParameters[9];

            rootParameters[0].InitAsConstantBufferView(0);
            rootParameters[1].InitAsConstantBufferView(1);
            rootParameters[2].InitAsConstantBufferView(2);

            CD3DX12_DESCRIPTOR_RANGE bindlessRange;
            bindlessRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 0, 0);
            rootParameters[3].InitAsDescriptorTable(1, &bindlessRange);

            rootParameters[4].InitAsShaderResourceView(0, 1);

            std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
            size_t count = 0;
            for (const ResourceBlockSignature& blockSignature : signature.mResourceBlocks) {
                count += blockSignature.mTypes.size();
            }
            ranges.resize(count);

            size_t total = 0;
            for (size_t i = 0; i < signature.mResourceBlocks.size(); ++i) {
                size_t count = signature.mResourceBlocks[i].mTypes.size();
                for (size_t j = 0; j < count; ++j) {
                    D3D12_DESCRIPTOR_RANGE_TYPE type;
                    switch (signature.mResourceBlocks[i].mTypes[j]) {
                    case ResourceBlockType::ConstantBuffer:
                        type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                        break;
                    case ResourceBlockType::StructuredBuffer:
                        type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        break;
                    case ResourceBlockType::Texture:
                        type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        break;
                    default:
                        throw 0;
                    }
                    ranges[total + j].Init(type, 1, j, i + 2);
                }
                rootParameters[5 + i].InitAsDescriptorTable(count, ranges.data() + total);
                total += count;
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

            rootSignatureDesc.Init(5 + signature.mResourceBlocks.size(), rootParameters, 2, samplerDesc, rootSignatureFlags);

            ReleasePtr<ID3DBlob> signature;
            ReleasePtr<ID3DBlob> error;
            HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
            DX12_CHECK(hr);
            hr = GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&it->second));
            DX12_CHECK(hr);
        }
        return it->second;
    }

    void DirectX12RenderContext::setupRootSignature(ID3D12RootSignature *signature, ID3D12GraphicsCommandList *list)
    {
        list->SetGraphicsRootSignature(signature);
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

        mGraphicsQueue.waitForIdle();
        mComputeQueue.waitForIdle();
        mCopyQueue.waitForIdle();
    }

    bool DirectX12RenderContext::supportsMultisampling() const
    {
        return true;
    }

    GPUPtr<void> DirectX12RenderContext::allocateBufferImpl(size_t size, UsageHint hint)
    {
        Block allocation = mBufferAllocator.allocate(size);

        if (!allocation.mAddress)
            return {};

        return { allocation.mAddress, size, [=, this](void *address) { mBufferAllocator.deallocate(allocation); } };
    }

    GPUPtr<Void[]> Engine::Render::DirectX12RenderContext::allocateBufferImpl(size_t elementSize, size_t count, UsageHint hint)
    {
        Block allocation = mBufferAllocator.allocate(elementSize * count);

        if (!allocation.mAddress)
            return {};

        return { allocation.mAddress, elementSize, count, [=, this](void *address) { mBufferAllocator.deallocate(allocation); } };
    }

    WritableByteBuffer DirectX12RenderContext::mapBufferImpl(const GPUPtr<void> &buffer)
    {
        Block uploadAllocation = mUploadAllocator.allocate(buffer.size());

        struct Deleter {
            void operator()(void *ptr)
            {
                auto [resource, offset] = mContext->mBufferMemoryHeap.resolve(mGPUAddress.get());

                auto list = mContext->mGraphicsQueue.fetchCommandList();
                list.Transition(resource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
                auto [heap, heapOffset] = mContext->mUploadHeap.resolve(ptr);
                list->CopyBufferRegion(resource, offset, heap, heapOffset, mGPUAddress.size());
                list.Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

                list.attachResource(std::move(mGPUAddress));
                list.execute();
            }

            GPUPtr<void> mGPUAddress;
            DirectX12RenderContext *mContext;
        };

        return {
            std::unique_ptr<void, Deleter> { uploadAllocation.mAddress, {
                                                                            buffer,
                                                                            this,
                                                                        } },
            buffer.size()
        };
    }

     WritableByteBuffer DirectX12RenderContext::mapBufferImpl(const GPUPtr<Void[]> &buffer)
    {
        Block uploadAllocation = mUploadAllocator.allocate(buffer.size());

        struct Deleter {
            void operator()(void *ptr)
            {
                auto [resource, offset] = mContext->mBufferMemoryHeap.resolve(mGPUAddress.get());

                auto list = mContext->mGraphicsQueue.fetchCommandList();
                list.Transition(resource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
                auto [heap, heapOffset] = mContext->mUploadHeap.resolve(ptr);
                list->CopyBufferRegion(resource, offset, heap, heapOffset, mGPUAddress.size());
                list.Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

                list.attachResource(std::move(mGPUAddress));
                list.execute();
            }

            GPUPtr<Void[]> mGPUAddress;
            DirectX12RenderContext *mContext;
        };

        return {
            std::unique_ptr<void, Deleter> { uploadAllocation.mAddress, {
                                                                            buffer,
                                                                            this,
                                                                        } },
            buffer.size()
        };
    }

    UniqueResourceBlock DirectX12RenderContext::createResourceBlock(std::vector<std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>>> data)
    {
        std::unique_ptr<DirectX12ResourceBlock<4>> block = std::make_unique<DirectX12ResourceBlock<4>>();

        OffsetPtr offset = mDescriptorHeap.allocate(data.size());

        block->mHandle = mDescriptorHeap.gpuHandle(offset);
        block->mSize = data.size();

        for (size_t i = 0; i < data.size(); ++i) {
            std::visit(overloaded {
                           [=, this, &block](const ConstTexturePtr &tex) {
                               if (tex) {
                                   std::static_pointer_cast<const DirectX12Texture>(tex)->createShaderResourceView(offset + i);
                                   block->mResources[i] = std::static_pointer_cast<const DirectX12Texture>(tex)->resourcePtr();
                               } else {
                                   D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc {};
                                   shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                                   shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                                   shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

                                   shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
                                   shaderResourceViewDesc.Texture2D.MipLevels = 1;
                                   shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

                                   GetDevice()->CreateShaderResourceView(nullptr, &shaderResourceViewDesc, mDescriptorHeap.cpuHandle(offset + i));
                               }
                           },
                           [&](const GPUPtr<void> &buf) {
                               auto [resource, resOffset] = mBufferMemoryHeap.resolve(buf.get());

                               D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc {};
                               cbvDesc.BufferLocation = resource->GetGPUVirtualAddress() + resOffset;
                               cbvDesc.SizeInBytes = alignTo(buf.size(), 256);

                               GetDevice()->CreateConstantBufferView(&cbvDesc, mDescriptorHeap.cpuHandle(offset + i));

                               block->mResources[i] = buf;
                           },
                           [&](const GPUPtr<Void[]> &buf) {
                               auto [resource, resOffset] = mBufferMemoryHeap.resolve(buf.get());

                               D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
                               srvDesc.Format = DXGI_FORMAT_UNKNOWN;
                               srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                               srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                               srvDesc.Buffer.FirstElement = resOffset;
                               srvDesc.Buffer.NumElements = buf.elementCount();
                               srvDesc.Buffer.StructureByteStride = buf.elementSize();

                               GetDevice()->CreateShaderResourceView(resource, &srvDesc, mDescriptorHeap.cpuHandle(offset + i));

                               block->mResources[i] = buf;
                           } },
                data[i]);            
        }

        UniqueResourceBlock result;

        result.setupAs<std::unique_ptr<DirectX12ResourceBlock<4>>>() = std::move(block);

        return result;
    }

    void DirectX12RenderContext::destroyResourceBlock(UniqueResourceBlock &block)
    {
        std::unique_ptr<DirectX12ResourceBlock<4>> data = block.release<std::unique_ptr<DirectX12ResourceBlock<4>>>();
        mDescriptorHeap.deallocate(data->mHandle);
    }

    TexturePtr DirectX12RenderContext::createTexture(TextureType type, TextureFormat format, Vector2i size, const ByteBuffer &data)
    {
        return std::make_shared<DirectX12Texture>(type, false, format, size, 1, data);
    }

    void DirectX12RenderContext::setTextureSubData(const TexturePtr &tex, Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
        static_cast<DirectX12Texture &>(*tex).setSubData(offset, size, data);
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
