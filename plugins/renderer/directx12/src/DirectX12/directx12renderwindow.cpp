#include "directx12lib.h"

#include "directx12renderwindow.h"

#include "Modules/debug/profiler/profile.h"

#include "Meta/math/vector2i.h"

#include "Interfaces/window/windowapi.h"

#include "directx12rendercontext.h"
#include "util/directx12commandallocator.h"

#include "Modules/threading/taskqueue.h"

namespace Engine {
namespace Render {

    DirectX12RenderWindow::DirectX12RenderWindow(DirectX12RenderContext *context, Window::OSWindow *w, size_t samples)
        : DirectX12RenderTarget(context, true, w->title(), TextureType_2D, samples)
        , mWindow(w)
    {
        //context->waitForGPU();

        HRESULT hr;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = w->renderSize().x;
        swapChainDesc.BufferDesc.Height = w->renderSize().y;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate = /*QueryRefreshRate(clientWidth, clientHeight, vSync)*/ { 60, 1 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = reinterpret_cast<HWND>(w->mHandle);
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

        InterfacesVector size = w->renderSize();

        std::vector<std::array<OffsetPtr, 6>> views;
        views.emplace_back()[0] = mCachedTargetViews[0];
        setup(std::move(views), { size.x, size.y });
    }

    DirectX12RenderWindow::~DirectX12RenderWindow()
    {
        shutdown();
    }

    bool DirectX12RenderWindow::skipFrame()
    {
        if (mWindow->isMinimized())
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

        //mCommandList.attachResource(mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()]);
    }

    RenderFuture DirectX12RenderWindow::endFrame()
    {
        DirectX12RenderTarget::endFrame();

        mCommandList.Transition(mBackBuffers[mSwapChain->GetCurrentBackBufferIndex()], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        mCommandList.execute();

        mSwapChain->Present(0, 0);

        return context()->graphicsQueue()->signalFence();
    }

    void DirectX12RenderWindow::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void DirectX12RenderWindow::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    Vector2i DirectX12RenderWindow::size() const
    {
        InterfacesVector size = mWindow->renderSize();
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

    bool DirectX12RenderWindow::resizeImpl(const Vector2i &size)
    {
        mTargetViews.clear();
        mResizeFence = context()->graphicsQueue()->currentFence();
        mResizeTarget = size;

        return true;
    }
}
}