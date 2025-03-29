#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Render {

    struct RenderDebuggable {
        virtual void debugFrustums(Closure<void(const Frustum &, std::string_view)> handler) const { }
        virtual void debugCameras(Closure<void(const Camera &, std::string_view)> handler) const { }
    };

}
}