#include "../renderlib.h"

#include "rendercontext.h"

#include "Generic/container/safeiterator.h"

#include "Modules/threading/workgroupstorage.h"

#include "rendertarget.h"

#include "Madgine/fontloader/fontloader.h"

#include "Modules/debug/profiler/profile.h"

namespace Engine {
namespace Render {

    Threading::WorkgroupLocal<RenderContext *> sContext = nullptr;

    RenderContext::RenderContext(Threading::TaskQueue *queue)
        : mRenderQueue(queue)
        , mRenderThread(std::this_thread::get_id())
    {
        assert(!sContext);
        sContext = this;
    }

    RenderContext::~RenderContext()
    {
        assert(mRenderTargets.empty());
        assert(sContext == this);
        sContext = nullptr;
    }

    Threading::Task<void> RenderContext::unloadAllResources()
    {
        for (std::pair<const std::string, FontLoader::Resource> &res : FontLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }
    }

    void RenderContext::addRenderTarget(RenderTarget *target)
    {
        mRenderTargets.push_back(target);
    }

    void RenderContext::removeRenderTarget(RenderTarget *target)
    {
        std::erase(mRenderTargets, target);
    }

    std::vector<const RenderTarget *> RenderContext::renderTargets() const
    {
        return { mRenderTargets.begin(), mRenderTargets.end() };
    }

    Threading::Task<void> RenderContext::render()
    {
        if (beginFrame()) {
            PROFILE();
            std::vector<Threading::TaskFuture<RenderFuture>> targets;
            for (RenderTarget *target : safeIterate(mRenderTargets))
                targets.push_back(target->update(this));
            for (Threading::TaskFuture<RenderFuture> &wait : targets)
                co_await wait;
            endFrame();
        }
    }

    bool RenderContext::beginFrame()
    {
        ++mFrame;
        return true;
    }

    void RenderContext::endFrame()
    {
    }

    size_t RenderContext::frame() const
    {
        return mFrame;
    }

    void RenderContext::checkThread()
    {
        assert(mRenderThread == std::this_thread::get_id());
    }

    Threading::TaskQueue *RenderContext::renderQueue()
    {
        return sContext->mRenderQueue;
    }

    bool RenderContext::isRenderThread()
    {
        assert(sContext);
        return sContext->mRenderThread == std::this_thread::get_id();
    }

    RenderContext &RenderContext::getSingleton()
    {
        return *sContext;
    }

}
}