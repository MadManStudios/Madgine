#pragma once

namespace Engine {
namespace Render {

typedef int RenderPassFlags;
    enum RenderPassFlags_ {
        RenderPassFlags_None = 0,
        RenderPassFlags_NoLighting = 1 << 0,
        RenderPassFlags_NoPerspective = 1 << 1,
        RenderPassFlags_DistanceField = 1 << 2
    };

}
}