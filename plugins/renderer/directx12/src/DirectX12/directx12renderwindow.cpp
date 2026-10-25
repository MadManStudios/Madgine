#include "directx12lib.h"

#include "directx12renderwindow.h"

#include "Generic/areaview.h"

#include "Platform/window/windowapi.h"

#include "Meta/math/vector2i.h"

#include "Modules/debug/profiler/profile.h"
#include "Modules/threading/taskqueue.h"

#include "directx12rendercontext.h"
#include "util/directx12commandallocator.h"

namespace Engine {
namespace Render {

    DirectX12RenderWindow::DirectX12RenderWindow(DirectX12RenderContext *context, Platform::Window::OSWindow *w, size_t samples)
        : DirectX12RenderTarget(context, true, w->title(), TextureType_2D, samples)
        , mWindow(w)
    {
        // context->waitForGPU();

        HRESULT hr;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = w->renderSize().x;
        swapChainDesc.BufferDesc.Height = w->renderSize().y;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate = /*QueryRefreshRate(clientWidth, clientHeight, vSync)*/ { 60, 1 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = (HWND)(w->ptrHandle());
        swapChainDesc.SampleDesc.Count = samples;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Windowed = TRUE;

        IDXGISwapChain *swapChain;
        hr = context->mFactory->CreateSwapChain(context->graphicsQueue()->queue(), &swapChainDesc, &swapChain);
        DX12_CHECK(hr);

        hr = swapChain->QueryInterface(&mSwapChain);
        DX12_CHECK(hr);

        mCachedTargetViews[0] = context->mRenderTargetDescriptorHeap.allocate();
        mCachedTargetViews[1] = context->mRenderTargetDescriptorHeap.allocate();

        createRenderTargetViews();

        // The Direct3D device and swap chain were successfully created.
        // Now we need to initialize the buffers of the swap chain.
        // Next initialize the back buffer of the swap chain and associate it to a
        // render target view.

        Platform::PlatformVector size = w->renderSize();

        std::vector<std::array<OffsetPtr, 6>> views;
        views.emplace_back()[0] = mCachedTargetViews[0];
        setup(std::move(views), { size.x, size.y });
    }

    DirectX12RenderWindow::~DirectX12RenderWindow()
    {
    }

    bool DirectX12RenderWindow::skipFrame()
    {
        if (mWindow->isMinimized() || RenderTarget::skipFrame())
            return true;
        if (mTargetViews.empty() && context()->graphicsQueue()->isComplete(mResizeFence)) {

            for (size_t i = 0; i < 2; ++i) {
                mBackBuffers[i].reset();
            }

            HRESULT hr = mSwapChain->ResizeBuffers(2, mResizeTarget.x, mResizeTarget.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            DX12_CHECK(hr);

            createRenderTargetViews();

            std::vector<std::array<OffsetPtr, 6>> views;
            views.emplace_back()[0] = mCachedTargetViews[0];
            setup(std::move(views), mResizeTarget);
        }
        return mTargetViews.empty();
    }

    void DirectX12RenderWindow::beginFrame()
    {
        mTargetViews[0][0] = mCachedTargetViews[mSwapChain->GetCurrentBackBufferIndex()];

        mCommandList = context()->fetchCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

        mCommandList.Transition(mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        DirectX12RenderTarget::beginFrame();

        // mCommandList.attachResource(mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()]);
    }

    RenderFuture DirectX12RenderWindow::endFrame()
    {
        DirectX12RenderTarget::endFrame();

        mCommandList.Transition(mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        mCommandList.execute();

        mSwapChain->Present(0, 0);

        return context()->graphicsQueue()->signalFence();
    }

    void DirectX12RenderWindow::readBack(Closure<void(AreaView<const uint32_t, 2> &)> cb)
    {
        ID3D12Resource *backBuffer = mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()];

        D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        UINT numRows;
        UINT64 rowSize;
        UINT64 totalSize;
        GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, &layout, &numRows, &rowSize, &totalSize);

        auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
        Platform::ReleasePtr<ID3D12Resource> readbackHeap;
        HRESULT hr = GetDevice()->CreateCommittedResource(
            &heapDesc,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&readbackHeap));
        DX12_CHECK(hr);

        mCommandList.Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

        CD3DX12_TEXTURE_COPY_LOCATION dst { readbackHeap, layout };
        CD3DX12_TEXTURE_COPY_LOCATION src { backBuffer, 0 };
        mCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        mCommandList.Transition(backBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

        struct ReadbackCallback {

            ReadbackCallback(Platform::ReleasePtr<ID3D12Resource> resource, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &footprint, Closure<void(AreaView<const uint32_t, 2> &)> cb)
                : mResource(std::move(resource))
                , mLayout(footprint)
                , mCallback(std::move(cb))
            {
            }
            ReadbackCallback(ReadbackCallback &&) = default;
            ~ReadbackCallback()
            {
                if (mResource) {
                    Math::Vector2i size { static_cast<int>(mLayout.Footprint.Width), static_cast<int>(mLayout.Footprint.Height) };
                    void *data = nullptr;
                    // The whole buffer: its last row is not padded to the row pitch
                    D3D12_RANGE range { 0, static_cast<SIZE_T>(mResource->GetDesc().Width) };
                    HRESULT hr = mResource->Map(0, &range, &data);
                    DX12_CHECK(hr);
                    AreaView<const uint32_t, 2> view = AreaView<const uint32_t, 2> { reinterpret_cast<const uint32_t *>(static_cast<const std::byte *>(data) + mLayout.Offset), { mLayout.Footprint.RowPitch / sizeof(uint32_t), static_cast<size_t>(size.y) } }.subArea({ 0, 0 }, { static_cast<size_t>(size.x), static_cast<size_t>(size.y) });
                    mCallback(view);
                    mResource->Unmap(0, nullptr);
                }
            }

            Platform::ReleasePtr<ID3D12Resource> mResource;
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT mLayout;
            Closure<void(AreaView<const uint32_t, 2> &)> mCallback;
        };

        mCommandList.attachResource(ReadbackCallback { std::move(readbackHeap), layout, std::move(cb) });
    }

    void DirectX12RenderWindow::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void DirectX12RenderWindow::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    Math::Vector2i DirectX12RenderWindow::size() const
    {
        Platform::PlatformVector size = mWindow->renderSize();
        return { size.x, size.y };
    }

    size_t DirectX12RenderWindow::textureCount() const
    {
        return 1;
    }

    TextureFormat DirectX12RenderWindow::textureFormat(size_t index) const
    {
        return FORMAT_RGBA8_SRGB;
    }

    void DirectX12RenderWindow::createRenderTargetViews()
    {
        for (size_t i = 0; i < 2; ++i) {
            HRESULT hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));
            DX12_CHECK(hr);

            assert(mBackBuffers[i]);

            mBackBuffers[i]->SetName((L"Window - BackBuffer " + std::to_wstring(i)).c_str());

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            GetDevice()->CreateRenderTargetView(mBackBuffers[i], &rtvDesc, DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.cpuHandle(mCachedTargetViews[i]));
        }
    }

    bool DirectX12RenderWindow::resizeImpl(const Math::Vector2i &size)
    {
        mTargetViews.clear();
        mResizeFence = context()->graphicsQueue()->currentFence();
        mResizeTarget = size;

        return true;
    }
}
}