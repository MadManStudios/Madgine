#pragma once

#include "../renderpass.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT ReadBackPass : RenderPass {

        ReadBackPass(int priority);

        virtual void setup(RenderTarget *target) override;
        virtual void render(RenderTarget *target, size_t iteration) override;
        virtual void onTargetResize(const Math::Vector2i &size) override;

        virtual int priority() const override;

        virtual std::string_view name() const override;

        void requestReadback(Closure<void(AreaView<const uint32_t, 2> &)> cb);

    private:
        int mPriority;
        
        std::mutex mMutex;
        std::vector<Closure<void(AreaView<const uint32_t, 2> &)>> mCallbacks;
    };

}
}