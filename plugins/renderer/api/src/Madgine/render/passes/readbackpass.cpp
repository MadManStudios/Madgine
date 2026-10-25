#include "../../renderlib.h"

#include "readbackpass.h"

#include "Generic/areaview.h"

#include "Meta/math/matrix4.h"

#include "Madgine/render/texture.h"
#include "Madgine/render/texturedescriptor.h"

#include "../rendercontext.h"
#include "../rendertarget.h"

namespace Engine {
namespace Render {

    ReadBackPass::ReadBackPass(int priority)
        : mPriority(priority)
    {
    }

    void ReadBackPass::setup(RenderTarget *target)
    {
    }

    void ReadBackPass::render(RenderTarget *target, size_t iteration)
    {
        std::vector<Closure<void(AreaView<const uint32_t, 2> &)>> callbacks;
        {
            std::unique_lock lock { mMutex };
            std::swap(mCallbacks, callbacks);
        }
        if (!callbacks.empty()) {

            target->readBack([callbacks { std::move(callbacks) }](AreaView<const uint32_t, 2> &view) {       
                for (const Closure<void(AreaView<const uint32_t, 2> &)>& cb : callbacks) {
                    cb(view);
                }
            });

        }
    }

    void ReadBackPass::onTargetResize(const Math::Vector2i &size)
    {
    }

    int ReadBackPass::priority() const
    {
        return mPriority;
    }

    std::string_view ReadBackPass::name() const
    {
        return "ReadBack";
    }

    void ReadBackPass::requestReadback(Closure<void(AreaView<const uint32_t, 2> &)> cb)
    {
        std::unique_lock lock { mMutex };
        mCallbacks.push_back(std::move(cb));
    }

}
}