#pragma once

#include "Modules/threading/task.h"

#include "Madgine/render/future.h"

namespace Engine {

namespace Render {

    struct MADGINE_RENDER_EXPORT RenderData {

        Threading::TaskFuture<RenderFuture> update(RenderContext *context);

        RenderFuture lastFrame() const;

    protected:
        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) = 0;

        virtual Threading::TaskQueue *queue(RenderContext *context);

        size_t frame() const;

    private:
        size_t mFrame = 0;
        Threading::TaskFuture<RenderFuture> mLastFrame;
    };

}

}