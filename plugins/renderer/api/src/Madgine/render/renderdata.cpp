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

            mLastFrame = queue(context)->queueTask(render(context));
        }

        return mLastFrame;
    }

    RenderFuture RenderData::lastFrame() const
    {
        if (!mLastFrame.valid())
            return {};
        return mLastFrame;
    }

    size_t RenderData::frame() const
    {
        return mFrame;
    }

    Threading::TaskQueue *RenderData::queue(RenderContext *context)
    {
        return context->renderQueue();
    }
}
}