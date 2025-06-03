#include "../renderlib.h"

#include "rendercontext.h"
#include "renderpass.h"
#include "rendertarget.h"

#include "Meta/math/matrix4.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/render/texture.h"

METATABLE_BEGIN(Engine::Render::RenderTarget)
METATABLE_END(Engine::Render::RenderTarget)

namespace Engine {
namespace Render {

    RenderTarget::RenderTarget(RenderContext *context, bool global, std::string name, bool flipFlop, RenderTarget *blitSource)
        : mGlobal(global)
        , mName(std::move(name))
        , mFlipFlop(flipFlop)
        , mBlitSource(blitSource)
        , mContext(context)
    {
        if (global)
            context->addRenderTarget(this);
    }

    RenderTarget::~RenderTarget()
    {
        for (RenderPass *pass : mRenderPasses) {
            pass->shutdown(this);
        }
        if (mGlobal)
            context()->removeRenderTarget(this);
    }

    Threading::ImmediateTask<RenderFuture> RenderTarget::render(RenderContext *context)
    {
        if (skipFrame())
            co_return {};

        std::vector<Threading::TaskFuture<RenderFuture>> dependencies;

        if (mBlitSource)
            dependencies.push_back(mBlitSource->update(context));

        for (RenderPass *pass : mRenderPasses)
            pass->preRender(dependencies, context);

        for (Threading::TaskFuture<RenderFuture> &dependency : dependencies)
            co_await dependency;

        LOG_DEBUG(mName << ": Begin Frame");

        beginFrame();

        for (RenderPass *pass : mRenderPasses) {
            pushAnnotation(pass->name().data());
            for (size_t iteration = 0; iteration < pass->iterations(); ++iteration) {
                size_t index = pass->targetIndex(iteration);
                size_t count = pass->targetCount(iteration);
                size_t subIndex = pass->targetSubresourceIndex(iteration);
                beginIteration(index, count, subIndex);
                pass->render(this, iteration);
                endIteration(index, count, subIndex);
                if (pass->swapFlipFlopTextures(iteration)) {
                    flipTextures(index, count);
                }
            }
            popAnnotation();
        }

        RenderFuture result = endFrame();

        LOG_DEBUG(mName << ": End Frame");

        co_return result;
    }

    void RenderTarget::addRenderPass(RenderPass *pass)
    {
        mRenderPasses.insert(
            std::upper_bound(mRenderPasses.begin(), mRenderPasses.end(), pass,
                [](RenderPass *first, RenderPass *second) { return first->priority() < second->priority(); }),
            pass);
        pass->setup(this);
    }

    void RenderTarget::removeRenderPass(RenderPass *pass)
    {
        pass->shutdown(this);
        std::erase(mRenderPasses, pass);
    }

    const std::vector<RenderPass *> &RenderTarget::renderPasses()
    {
        return mRenderPasses;
    }

    std::vector<const RenderPass *> RenderTarget::renderPasses() const
    {
        return { mRenderPasses.begin(), mRenderPasses.end() };
    }

    bool RenderTarget::skipFrame()
    {
        return false;
    }

    void RenderTarget::beginFrame()
    {
        pushAnnotation(mName.empty() ? "<unnamed target>" : mName.c_str());
    }

    RenderFuture RenderTarget::endFrame()
    {
        popAnnotation();
        return {};
    }

    void RenderTarget::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
    }

    void RenderTarget::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
    }

    bool RenderTarget::canFlipFlop() const
    {
        return mFlipFlop;
    }

    const std::string &RenderTarget::name() const
    {
        return mName;
    }

    bool RenderTarget::resize(const Vector2i &size)
    {
        if (mBlitSource)
            mBlitSource->resize(size);
        bool resized = resizeImpl(size);
        if (resized) {
            for (RenderPass *pass : mRenderPasses)
                pass->onTargetResize(size);
        }
        return resized;
    }

    Matrix4 RenderTarget::getClipSpaceMatrix() const
    {
        return Matrix4::IDENTITY;
    }

    const Texture *RenderTarget::texture(size_t index) const
    {
        return nullptr;
    }

    size_t RenderTarget::textureCount() const
    {
        return 0;
    }

    const Texture *RenderTarget::depthTexture() const
    {
        return nullptr;
    }

    RenderContext *RenderTarget::context() const
    {
        return mContext;
    }

    const RenderTarget *RenderTarget::blitSource() const
    {
        return mBlitSource;
    }

    void RenderTarget::flipTextures(size_t startIndex, size_t count)
    {
        assert(canFlipFlop());
        for (size_t i = 0; i < count; ++i)
            mFlipFlopIndices[startIndex + i] = mFlipFlopIndices[startIndex + i] ^ 1;
    }

    void RenderTarget::addPostProcessing(std::string_view processing)
    {
        mPostProcessing.emplace_back(processing);
    }

    const std::vector<std::string> &RenderTarget::postProcessing() const
    {
        return mPostProcessing;
    }

}
}