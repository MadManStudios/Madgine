#pragma once

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"

#include "Generic/bytebuffer.h"

#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12Texture : Texture {

        DirectX12Texture(TextureType type, bool isRenderTarget, TextureFormat format, Vector2i size, size_t samples = 1, const ByteBuffer &data = {});
        DirectX12Texture(TextureType type = TextureType_2D, bool isRenderTarget = false, TextureFormat format = FORMAT_RGBA8, size_t samples = 1);
        DirectX12Texture(const DirectX12Texture &) = delete;
        DirectX12Texture(DirectX12Texture &&);
        ~DirectX12Texture();

        DirectX12Texture &operator=(DirectX12Texture &&);

        void reset();

        void setData(Vector2i size, const ByteBuffer &data);
        void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data);

        void createShaderResourceView(OffsetPtr descriptorHandle) const;

        operator ID3D12Resource *() const;     
        operator ReleasePtr<ID3D12Resource>() const;

        void setName(std::string_view name);

        D3D12_RESOURCE_STATES readStateFlags() const;

        size_t samples() const;

    private:
        bool mIsRenderTarget;
        size_t mSamples = 0;
    };

}
}