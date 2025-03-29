#include "directx12lib.h"

#include "directx12rendertexture.h"

#include "directx12rendercontext.h"

namespace Engine {
namespace Render {

    DirectX12RenderTexture::DirectX12RenderTexture(DirectX12RenderContext *context, const Vector2i &size, const RenderTextureConfig &config)
        : DirectX12RenderTarget(context, false, config.mName, config.mType, config.mSamples, config.mFlipFlop, config.mBlitSource)
    {
        size_t bufferCount = config.mFlipFlop ? 2 : 1;

        for (size_t i = 0; i < config.mTextureCount * bufferCount; ++i) {
            mTextures.emplace_back(config.mType, true, config.mFormat, config.mSamples);
        }

        size_t count = config.mType == TextureType_Cube ? 6 : 1;
        mTargetViews.resize(mTextures.size());
        for (std::array<OffsetPtr, 6> &ptr : mTargetViews)
            for (size_t i = 0; i < count; ++i)
                ptr[i] = DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.allocate();

        resize(size);
    }

    DirectX12RenderTexture::~DirectX12RenderTexture()
    {
        shutdown();
    }

    bool DirectX12RenderTexture::resizeImpl(const Vector2i &size)
    {
        if (mDepthTexture.size() == size)
            return false;

        if (context()->graphicsQueue()->isComplete(lastFrame())) {
            resizeBuffers(size);
        } else {
            if (mResizePending && mResizeTarget == size)
                return false;

            mResizePending = true;
            mResizeFence = context()->graphicsQueue()->currentFence();
            mResizeTarget = size;
        }

        return true;
    }

    void DirectX12RenderTexture::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void DirectX12RenderTexture::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        DirectX12RenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    const DirectX12Texture *DirectX12RenderTexture::texture(size_t index) const
    {
        int offset = canFlipFlop() ? (1 ^ mFlipFlopIndices[index]) : 0;
        return &mTextures[textureCount() * offset + index];
    }

    size_t DirectX12RenderTexture::textureCount() const
    {
        int bufferCount = canFlipFlop() ? 2 : 1;
        return mTextures.size() / bufferCount;
    }

    const DirectX12Texture *DirectX12RenderTexture::depthTexture() const
    {
        return &mDepthTexture;
    }

    const std::vector<DirectX12Texture> &DirectX12RenderTexture::textures() const
    {
        return mTextures;
    }

    void DirectX12RenderTexture::resizeBuffers(const Vector2i &size)
    {
        size_t count = 1;
        size_t i = 0;
        std::vector<std::array<OffsetPtr, 6>> targetViews = std::move(mTargetViews);

        for (DirectX12Texture &tex : mTextures) {
            tex.setData(size, {});
            tex.setName((name().empty() ? "RenderTexture" : name()) + "[" + std::to_string(i) + "]");

            D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc {};

            switch (tex.format()) {
            case FORMAT_RGBA8:
                renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                break;
            case FORMAT_RGBA8_SRGB:
                renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                break;
            case FORMAT_RGBA16F:
                renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                break;
            default:
                throw 0;
            }
            switch (tex.type()) {
            case TextureType_2D:
                renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                break;
            case TextureType_2DMultiSample:
                renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                break;
            default:
                throw 0;
            }
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            for (size_t j = 0; j < count; ++j) {
                GetDevice()->CreateRenderTargetView(tex.resource(), &renderTargetViewDesc, DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.cpuHandle(targetViews[i][j]));
            }

            ++i;
        }

        setup(std::move(targetViews), size);
    }

    void DirectX12RenderTexture::flipTextures(size_t startIndex, size_t count)
    {        
        size_t size = textureCount();
        count = std::min(size, count);
        for (size_t index = 0; index < count; ++index) {
            int oldOffset = mFlipFlopIndices[index + startIndex];
            int newOffset = 1 ^ mFlipFlopIndices[index + startIndex];
            mCommandList.Transition(mTextures[size * oldOffset + index + startIndex].resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, mTextures[size * oldOffset + index + startIndex].readStateFlags());
            mCommandList.Transition(mTextures[size * newOffset + index + startIndex].resource(), mTextures[size * newOffset + index + startIndex].readStateFlags(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
        DirectX12RenderTarget::flipTextures(startIndex, count);
    }

    Vector2i DirectX12RenderTexture::size() const
    {
        return mDepthTexture.size();
    }

    bool DirectX12RenderTexture::skipFrame()
    {
        if (mResizePending && context()->graphicsQueue()->isComplete(mResizeFence)) {
            mResizePending = false;
            resizeBuffers(mResizeTarget);
        }

        return mResizePending;
    }

    void DirectX12RenderTexture::beginFrame()
    {
        mCommandList = context()->fetchCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

        for (DirectX12Texture &tex : mTextures)
            mCommandList.attachResource(tex.resourcePtr());

        int bufferCount = canFlipFlop() ? 2 : 1;
        size_t size = mTextures.size() / bufferCount;
        for (size_t index = 0; index < size; ++index) {
            int offset = mFlipFlopIndices[index];
            mCommandList.Transition(mTextures[size * offset + index].resource(), mTextures[size * offset + index].readStateFlags(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        }

        DirectX12RenderTarget::beginFrame();

        if (mBlitSource) {
            blit(mBlitSource);
            if (canFlipFlop()) {
                flipTextures(0, textureCount());
            }
        }
    }

    RenderFuture DirectX12RenderTexture::endFrame()
    {
        DirectX12RenderTarget::endFrame();

        int bufferCount = canFlipFlop() ? 2 : 1;
        size_t size = mTextures.size() / bufferCount;
        for (size_t index = 0; index < size; ++index) {
            int offset = mFlipFlopIndices[index];
            mCommandList.Transition(mTextures[size * offset + index].resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, mTextures[size * offset + index].readStateFlags());
        }

        return mCommandList.execute();
    }

    void DirectX12RenderTexture::blit(RenderTarget *input) const
    {
        DirectX12RenderTexture *inputTex = dynamic_cast<DirectX12RenderTexture *>(input);
        assert(inputTex);

        size_t count = std::min(textureCount(), inputTex->textureCount());

        for (size_t i = 0; i < count; ++i) {
            DXGI_FORMAT xFormat;
            switch (mTextures[i].format()) {
            case FORMAT_RGBA8:
                xFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
                break;
            case FORMAT_RGBA8_SRGB:
                xFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                break;
            case FORMAT_RGBA16F:
                xFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
                break;
            case FORMAT_D24:
                xFormat = DXGI_FORMAT_R24G8_TYPELESS;
                break;
            case FORMAT_D32:
                xFormat = DXGI_FORMAT_R32_TYPELESS;
                break;
            default:
                std::terminate();
            }

            int offset = canFlipFlop() ? mFlipFlopIndices[i] : 0;
            const DirectX12Texture &target = mTextures[textureCount() * offset + i];
            mCommandList.Transition(target.resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            mCommandList->ResolveSubresource(target.resource(), 0, inputTex->texture(i)->resource(), 0, xFormat);
            mCommandList.Transition(target.resource(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
    }

}
}
