#include "directx12lib.h"

#include "directx12rendertarget.h"

#include "Madgine/meshloader/meshdata.h"

#include "directx12meshdata.h"
#include "directx12meshloader.h"
#include "util/directx12pipelineinstance.h"

#include "directx12rendercontext.h"
#include "directx12rendertexture.h"

#include "Meta/math/rect2i.h"

#include "Madgine/render/renderpass.h"

namespace Engine {
namespace Render {

    constexpr FLOAT sClearColor[4] = { 0.033f, 0.073f, 0.073f, 1.0f };

    DirectX12RenderTarget::DirectX12RenderTarget(DirectX12RenderContext *context, bool global, std::string name, TextureType type, size_t samples, bool flipFlop, RenderTarget *blitSource)
        : RenderTarget(context, global, name, flipFlop, blitSource)
        , mDepthTexture(type, false, FORMAT_D24, samples)
        , mSamples(samples)
    {
    }

    DirectX12RenderTarget::~DirectX12RenderTarget()
    {
    }

    void DirectX12RenderTarget::setup(std::vector<std::array<OffsetPtr, 6>> targetViews, const Vector2i &size)
    {
        mTargetViews = targetViews;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        size_t count = 1;
        UINT *target = nullptr;
        switch (mDepthTexture.type()) {
        case TextureType_2D:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            break;
        case TextureType_2DMultiSample:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
            break;
        case TextureType_Cube:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.ArraySize = 1;
            count = 6;
            target = &dsvDesc.Texture2DArray.FirstArraySlice;
            break;
        default:
            throw 0;
        }

        mDepthTexture.setData(size, {});
        mDepthTexture.setName(name() + "DepthTexture");

        for (size_t i = 0; i < count; ++i) {
            if (!mDepthStencilViews[i])
                mDepthStencilViews[i] = DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.allocate();
            if (target)
                *target = i;
            GetDevice()->CreateDepthStencilView(mDepthTexture.resource(), &dsvDesc, DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilViews[i]));
            DX12_CHECK();
        }
    }

    void DirectX12RenderTarget::shutdown()
    {
    }

    void DirectX12RenderTarget::beginFrame()
    {
        RenderTarget::beginFrame();

        mCommandList.Transition(mDepthTexture.resource(), mDepthTexture.readStateFlags(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

        mCommandList.attachResource(mDepthTexture.resourcePtr());

        for (RenderPass *pass : renderPasses()) {
            for (RenderData *data : pass->dependencies()) {
                DirectX12RenderTexture *tex = dynamic_cast<DirectX12RenderTexture *>(data);
                if (tex) {
                    for (const DirectX12Texture &texture : tex->textures())
                        mCommandList.attachResource(texture.resourcePtr());
                }
                context()->mGraphicsQueue.wait(data->lastFrame());
            }
        }

        context()->setupRootSignature(mCommandList);

        const Vector2i &screenSize = size();

        D3D12_VIEWPORT viewport;
        viewport.Width = static_cast<float>(screenSize.x);
        viewport.Height = static_cast<float>(screenSize.y);
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        mCommandList->RSSetViewports(1, &viewport);

        D3D12_RECT scissorRect;
        scissorRect.left = 0.0f;
        scissorRect.top = 0.0f;
        scissorRect.right = static_cast<float>(screenSize.x);
        scissorRect.bottom = static_cast<float>(screenSize.y);

        mCommandList->RSSetScissorRects(1, &scissorRect);

        if (!mBlitSource) {
            int bufferCount = canFlipFlop() ? 2 : 1;
            size_t size = mTargetViews.size() / bufferCount;
            for (size_t index = 0; index < size; ++index) {
                int offset = mFlipFlopIndices[index];
                for (size_t i = 0; i < 6; ++i) {
                    if (mTargetViews[size * offset + index][i])
                        mCommandList->ClearRenderTargetView(DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.cpuHandle(mTargetViews[size * offset + index][i]), sClearColor, 0, nullptr);
                }
            }
            clearDepthBuffer();
        }
    }

    RenderFuture DirectX12RenderTarget::endFrame()
    {

        mCommandList.Transition(mDepthTexture.resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, mDepthTexture.readStateFlags());

        return RenderTarget::endFrame();
    }

    void DirectX12RenderTarget::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        RenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);

        D3D12_CPU_DESCRIPTOR_HANDLE targetViews[8] = { 0 };
        int bufferCount = canFlipFlop() ? 2 : 1;
        size_t size = mTargetViews.size() / bufferCount;
        size_t count = std::min(size, targetCount);
        for (size_t index = 0; index < count; ++index) {
            int offset = mFlipFlopIndices[index + targetIndex];
            targetViews[index] = DirectX12RenderContext::getSingleton().mRenderTargetDescriptorHeap.cpuHandle(mTargetViews[size * offset + index + targetIndex][targetSubresourceIndex]);
            if (canFlipFlop())
                mCommandList->ClearRenderTargetView(targetViews[index], sClearColor, 0, nullptr);
        }
        D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilViews[targetSubresourceIndex]);
        mCommandList->OMSetRenderTargets(count, targetViews, false, &depthStencilView);

        if (canFlipFlop())
            mCommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    void DirectX12RenderTarget::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        RenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void DirectX12RenderTarget::setRenderSpace(const Rect2i &space)
    {
        D3D12_VIEWPORT viewport;
        viewport.Width = static_cast<float>(space.mSize.x);
        viewport.Height = static_cast<float>(space.mSize.y);
        viewport.TopLeftX = static_cast<float>(space.mTopLeft.x);
        viewport.TopLeftY = static_cast<float>(space.mTopLeft.y);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        mCommandList->RSSetViewports(1, &viewport);

        D3D12_RECT scissorRect;
        scissorRect.left = static_cast<float>(space.mTopLeft.x);
        scissorRect.top = static_cast<float>(space.mTopLeft.y);
        scissorRect.right = static_cast<float>(space.mTopLeft.x + space.mSize.x);
        scissorRect.bottom = static_cast<float>(space.mTopLeft.y + space.mSize.y);

        mCommandList->RSSetScissorRects(1, &scissorRect);
    }

    void DirectX12RenderTarget::setScissorsRect(const Rect2i &space)
    {
        D3D12_RECT scissorRect;
        scissorRect.left = static_cast<float>(space.mTopLeft.x);
        scissorRect.top = static_cast<float>(space.mTopLeft.y);
        scissorRect.right = static_cast<float>(space.mTopLeft.x + space.mSize.x);
        scissorRect.bottom = static_cast<float>(space.mTopLeft.y + space.mSize.y);

        mCommandList->RSSetScissorRects(1, &scissorRect);
    }

    void DirectX12RenderTarget::pushAnnotation(const char *tag)
    {
        PIXBeginEvent(mCommandList, PIX_COLOR(255, 255, 255), tag);
    }

    void DirectX12RenderTarget::popAnnotation()
    {
        PIXEndEvent(mCommandList);
    }

    void DirectX12RenderTarget::clearDepthBuffer()
    {
        for (size_t i = 0; i < 6; ++i)
            if (mDepthStencilViews[i])
                mCommandList->ClearDepthStencilView(DirectX12RenderContext::getSingleton().mDepthStencilDescriptorHeap.cpuHandle(mDepthStencilViews[i]), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    TextureFormat DirectX12RenderTarget::textureFormat(size_t index) const
    {
        return texture(index)->format();
    }

    DirectX12RenderContext *DirectX12RenderTarget::context() const
    {
        return static_cast<DirectX12RenderContext *>(RenderTarget::context());
    }

    size_t DirectX12RenderTarget::samples() const
    {
        return mSamples;
    }

}
}
