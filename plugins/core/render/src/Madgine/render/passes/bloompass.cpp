#include "../../renderlib.h"

#include "bloompass.h"

#include "../rendercontext.h"
#include "../rendertarget.h"

#include "../shadinglanguage/sl_support_begin.h"
#include "shaders/bloom.sl"
#include "../shadinglanguage/sl_support_end.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/render/texture.h"

METATABLE_BEGIN(Engine::Render::BloomPass)
MEMBER(mExposure)
METATABLE_END(Engine::Render::BloomPass)

namespace Engine {
namespace Render {

    BloomPass::BloomPass(int priority, size_t blurIterations)
        : mPriority(priority)
        , mBlur(priority - 1, blurIterations)
    {
    }

    BloomPass::~BloomPass()
    {
    }

    void BloomPass::setup(RenderTarget *target)
    {
        mPipeline.create({ .vs = "bloom", .ps = "bloom", .bufferSizes = { sizeof(BloomData) } });

        target->addRenderPass(&mBlur);
    }

    void BloomPass::shutdown(RenderTarget *target)
    {
        mPipeline.reset();
    }

    void BloomPass::render(RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        mPipeline->bindResources(target, 2, mInput->texture(mInputIndex)->resource());
        mPipeline->bindResources(target, 3, target->texture(1)->resource());

        mPipeline->mapParameters<BloomData>(0)->exposure = mExposure;

        mPipeline->renderQuad(target);        
    }

    void BloomPass::onTargetResize(const Vector2i &size)
    {
        mInput->resize(size);
    }

    int BloomPass::priority() const
    {
        return mPriority;
    }

    bool BloomPass::swapFlipFlopTextures(size_t) const
    {
        return true;
    }

    std::string_view BloomPass::name() const
    {
        return "Bloom";
    }
    
    void BloomPass::setInput(RenderTarget *input, size_t inputIndex, RenderTarget *blurInput, size_t blurIndex)
    {
        mInput = input;
        //addDependency(input);
        mInputIndex = inputIndex;
        mBlur.setInput(blurInput ? blurInput : input, blurIndex);
    }

}
}