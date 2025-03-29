#pragma once

#include "directx12rendertarget.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12RenderWindow : DirectX12RenderTarget {
        DirectX12RenderWindow(DirectX12RenderContext *context, Window::OSWindow *w, size_t samples);
        DirectX12RenderWindow(const DirectX12RenderWindow &) = delete;
        ~DirectX12RenderWindow();

        virtual bool skipFrame() override;
        virtual void beginFrame() override;
        virtual RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual bool resizeImpl(const Vector2i &size) override;
        virtual Vector2i size() const override;

        virtual size_t textureCount() const override;
        virtual TextureFormat textureFormat(size_t index) const override;

    protected:
        void createRenderTargetViews();

    private:
        Window::OSWindow *mWindow;

        ReleasePtr<IDXGISwapChain3> mSwapChain;

        OffsetPtr mCachedTargetViews[2];
        ReleasePtr<ID3D12Resource> mBackBuffers[2];

        RenderFuture mResizeFence;
        Vector2i mResizeTarget;
    };

}
}

REGISTER_TYPE(Engine::Render::DirectX12RenderWindow)