#pragma once

#include "Madgine/render/texturedescriptor.h"
#include "Madgine/render/resourceblock.h"

#include "Madgine/imageloader/imageloader.h"

namespace Engine {
namespace Widgets {

    typedef int TextureFlags;
    enum TextureFlags_ {
        TextureFlag_IsDistanceField = 1 << 0
    };

    struct TextureSettings {
        Render::ResourceBlock mResource;
        TextureFlags mFlags = 0;

        auto operator<=>(const TextureSettings &other) const = default;
    };

}
}