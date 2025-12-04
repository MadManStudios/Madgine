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

    private:
        size_t mFrame = 0;
        Threading::TaskFuture<RenderFuture> mLastFrame;
    };

}

}