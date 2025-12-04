#include "../renderlib.h"

#include "renderdata.h"

#include "Modules/threading/taskqueue.h"

#include "rendercontext.h"

namespace Engine {
namespace Render {

    Threading::TaskFuture<RenderFuture> RenderData::update(RenderContext *context)
    {
        if (context->frame() != mFrame) {
            mFrame = context->frame();

            mLastFrame = context->renderQueue()->queueTask(render(context));
        }

        return mLastFrame;
    }

    RenderFuture RenderData::lastFrame() const
    {
        if (!mLastFrame.valid())
            return {};
        return mLastFrame;
    }

}
}