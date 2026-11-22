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

            mLastFrame = queue(context).queueTask(render(context));
        }

        return mLastFrame;
    }

    const Threading::TaskFuture<RenderFuture> &RenderData::lastFrame() const
    {
        return mLastFrame;
    }

    size_t RenderData::frame() const
    {
        return mFrame;
    }

    Threading::TaskQueue &RenderData::queue(RenderContext *context)
    {
        return context->renderQueue();
    }
}
}