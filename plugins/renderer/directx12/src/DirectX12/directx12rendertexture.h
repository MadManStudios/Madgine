#pragma once

#include "directx12rendertarget.h"
#include "util/directx12texture.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12RenderTexture : DirectX12RenderTarget {

        DirectX12RenderTexture(DirectX12RenderContext *context, const Vector2i &size, const RenderTextureConfig &config);
        ~DirectX12RenderTexture();

        bool resizeImpl(const Vector2i &size) override;
        Vector2i size() const override;

        virtual bool skipFrame() override;
        virtual void beginFrame() override;
        virtual RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual const DirectX12Texture *texture(size_t index) const override;
        virtual size_t textureCount() const override;
        virtual const DirectX12Texture *depthTexture() const override;

        void blit(RenderTarget *input) const;

        const std::vector<DirectX12Texture> &textures() const;

    protected:
        void resizeBuffers(const Vector2i &size);

        void flipTextures(size_t startIndex, size_t count) override;

    private:
        std::vector<DirectX12Texture> mTextures;

        RenderFuture mResizeFence;
        Vector2i mResizeTarget;
        bool mResizePending = false;
    };

}
}