#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"
#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    template <size_t I = 1>
    struct DirectX12ResourceBlock {
        D3D12_GPU_DESCRIPTOR_HANDLE mHandle;
        size_t mSize = I;
        std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>> mResources[I];
    };

    struct MADGINE_DIRECTX12_EXPORT DirectX12Texture : Texture {

        DirectX12Texture(TextureType type, bool isRenderTarget, TextureFormat format, Vector2i size, size_t samples = 1, const ByteBuffer &data = {});
        DirectX12Texture(TextureType type = TextureType_2D, bool isRenderTarget = false, TextureFormat format = FORMAT_RGBA8, size_t samples = 1);
        DirectX12Texture(const DirectX12Texture &) = delete;
        DirectX12Texture(DirectX12Texture &&);
        ~DirectX12Texture();

        DirectX12Texture &operator=(DirectX12Texture &&);

        void reset();

        void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data);

        void createShaderResourceView(OffsetPtr descriptorHandle) const;

        ID3D12Resource *resource() const;
        ReleasePtr<ID3D12Resource> resourcePtr() const;

        void setName(std::string_view name);

        D3D12_RESOURCE_STATES readStateFlags() const;

        size_t samples() const;

    private:
        bool mIsRenderTarget;
        size_t mSamples = 0;
        DirectX12ResourceBlock<1> mBlock;
    };

}
}