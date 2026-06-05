#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Render {

    struct RenderDebuggable {
        virtual void debugFrustums(CallableView<void(const Math::Frustum &, std::string_view)> handler) const { }
        virtual void debugCameras(CallableView<void(const Camera &, std::string_view)> handler) const { }
    };

}
}