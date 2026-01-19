#pragma once

namespace Engine {
namespace Render {

    struct Pipeline;
    struct GPUMeshData;
    struct Texture;
    struct Material;
    struct Glyph;
    struct Font;

    typedef int RenderPassFlags;
    struct TextureHandle;

    struct RenderFuture;

    struct ResourceBlock;
    struct UniqueResourceBlock;

    using ConstTexturePtr = std::shared_ptr<const Texture>;
    using TexturePtr = std::shared_ptr<Texture>;

    template <typename T>
    struct GPUPtr;

    struct ConstantValues;

}
}