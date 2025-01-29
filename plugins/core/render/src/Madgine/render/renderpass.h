#pragma once

#include "Modules/threading/taskfuture.h"
#include "renderdebuggable.h"
#include "Madgine/render/future.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT RenderPass : RenderDebuggable {
        virtual ~RenderPass() = default;

        virtual void setup(RenderTarget *target) { }
        virtual void shutdown(RenderTarget *target) { }
        virtual void render(RenderTarget *target, size_t iteration) = 0;
        void preRender(std::vector<Threading::TaskFuture<RenderFuture>> &dependencies, RenderContext *context);

        const std::vector<RenderData *> &dependencies() const;

        virtual void onTargetResize(const Vector2i &size) {};

        virtual int priority() const = 0;
        virtual size_t iterations() const;
        virtual size_t targetIndex(size_t iteration) const;
        virtual size_t targetCount(size_t iteration) const;
        virtual size_t targetSubresourceIndex(size_t iteration) const;
        virtual bool swapFlipFlopTextures(size_t iteration) const;

        virtual std::string_view name() const = 0;

    protected:
        void addDependency(RenderData *dep);
        void removeDependency(RenderData *dep);

    private:
        std::vector<RenderData *> mDependencies;
    };

}
}