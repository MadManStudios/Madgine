#pragma once

#include "../renderpass.h"

#include "../pipelineloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT BlurPass : RenderPass {

        BlurPass(int priority, size_t iterations = 10);
        
        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;
        virtual void render(RenderTarget *target, size_t iteration) override;
        virtual void onTargetResize(const Vector2i &size) override;

        virtual int priority() const override;
        virtual size_t iterations() const override;
        virtual bool swapFlipFlopTextures(size_t) const override;
        virtual size_t targetIndex(size_t) const override;

        virtual std::string_view name() const override;

        void setInput(RenderTarget *input, size_t inputIndex = 0);

    private:
        int mPriority; 
        size_t mIterations;

        size_t mInputIndex;
        RenderTarget *mInput;

        PipelineLoader::Instance mPipeline;
    };

}
}