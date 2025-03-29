#pragma once

#include "Madgine/render/rendertarget.h"

#include "util/directx12commandlist.h"

#include "util/directx12texture.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12RenderTarget : RenderTarget {

        DirectX12RenderTarget(DirectX12RenderContext *context, bool global, std::string name, TextureType type, size_t samples = 1, bool flipFlop = false, RenderTarget *blitSource = nullptr);
        ~DirectX12RenderTarget();

        void setup(std::vector<std::array<OffsetPtr, 6>> targetViews, const Vector2i &size);
        void shutdown();

        void beginFrame() override;
        RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual void pushAnnotation(const char *tag) override;
        virtual void popAnnotation() override;

        virtual void setRenderSpace(const Rect2i &space) override;
        virtual void setScissorsRect(const Rect2i &space) override;

        virtual void clearDepthBuffer() override;

        virtual TextureFormat textureFormat(size_t index = 0) const;

        DirectX12RenderContext *context() const;

        size_t samples() const;

        mutable DirectX12CommandList mCommandList;
        mutable std::vector<std::array<OffsetPtr, 6>> mTargetViews;
        OffsetPtr mDepthStencilViews[6];
        D3D12_DEPTH_STENCIL_DESC mDepthStencilStateDesc;
        D3D12_RASTERIZER_DESC mRasterizerDesc;
        D3D12_BLEND_DESC mBlendState;
        D3D12_RESOURCE_STATES mResourceState;

        size_t mSamples;

        DirectX12Texture mDepthTexture;
    };

}
}