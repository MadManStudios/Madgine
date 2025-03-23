#include "../../renderlib.h"

#include "blurpass.h"

#include "../rendertarget.h"

#include "Madgine/render/texturedescriptor.h"
#include "Madgine/render/texture.h"

#include "../rendercontext.h"

#include "../shadinglanguage/sl_support_begin.h"
#include "shaders/blur.sl"
#include "../shadinglanguage/sl_support_end.h"

namespace Engine {
namespace Render {

    BlurPass::BlurPass(int priority, size_t iterations)
        : mPriority(priority)
        , mIterations(iterations)
    {
    }

    void BlurPass::setup(RenderTarget *target)
    {
        mPipeline.create({ .vs = "blur", .ps = "blur", .bufferSizes = { sizeof(BlurData) } });
    }

    void BlurPass::shutdown(RenderTarget *target)
    {
        mPipeline.reset();
    }

    void BlurPass::render(RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        {
            auto data = mPipeline->mapParameters<BlurData>(0);
            data->horizontal = iteration % 2;
            data->textureSize = mInput->size();
        }

        if (iteration == 0) {
            mPipeline->bindResources(target, 2, mInput->texture(mInputIndex)->resource());
        } else {
            mPipeline->bindResources(target, 2, target->texture(1)->resource());
        }

        mPipeline->renderQuad(target);
    }

    void BlurPass::onTargetResize(const Vector2i &size)
    {
        mInput->resize(size);
    }

    int BlurPass::priority() const
    {
        return mPriority;
    }

    size_t BlurPass::iterations() const
    {
        return mIterations;
    }

    bool BlurPass::swapFlipFlopTextures(size_t) const
    {
        return true;
    }

    size_t BlurPass::targetIndex(size_t) const
    {
        return 1;
    }

    std::string_view BlurPass::name() const
    {
        return "Blur";
    }

    void BlurPass::setInput(RenderTarget *input, size_t inputIndex)
    {
        mInput = input;
        //addDependency(input);
        mInputIndex = inputIndex;
    }

}
}