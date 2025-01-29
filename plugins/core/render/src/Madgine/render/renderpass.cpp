#include "../renderlib.h"

#include "renderpass.h"

#include "renderdata.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Render::RenderPass)
METATABLE_END(Engine::Render::RenderPass)

namespace Engine {
namespace Render {

    void RenderPass::preRender(std::vector<Threading::TaskFuture<RenderFuture>> &dependencies, RenderContext *context)
    {        
        for (RenderData *dep : mDependencies)
            dependencies.push_back(dep->update(context));        
        for (const auto &fut : dependencies)
            assert(fut.valid());
    }

    const std::vector<RenderData *> &RenderPass::dependencies() const
    {
        return mDependencies;
    }

    void RenderPass::addDependency(RenderData *dep)
    {
        mDependencies.push_back(dep);
    }

    void RenderPass::removeDependency(RenderData *dep)
    {
        std::erase(mDependencies, dep);
    }

    size_t RenderPass::iterations() const
    {
        return 1;
    }

    size_t RenderPass::targetIndex(size_t iteration) const
    {
        return 0;
    }

    size_t RenderPass::targetCount(size_t iteration) const
    {
        return 1;
    }

    size_t RenderPass::targetSubresourceIndex(size_t iteration) const
    {
        return 0;
    }

    bool RenderPass::swapFlipFlopTextures(size_t iteration) const
    {
        return false;
    }

}
}