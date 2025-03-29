#pragma once

#include "Madgine/render/pipeline.h"

#include "../directx12pixelshaderloader.h"
#include "../directx12vertexshaderloader.h"

#include "Madgine/render/vertexformat.h"

#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    struct PipelineDescriptor {

        bool operator==(const PipelineDescriptor &) const = default;

        VertexFormat mVertexFormat;
        size_t mGroupSize;
        std::vector<TextureFormat> mTextureFormats;
        size_t mSamples;
    };
}
}

namespace std {
template <>
struct hash<Engine::Render::PipelineDescriptor> {
    std::size_t operator()(const Engine::Render::PipelineDescriptor &desc) const noexcept
    {
        std::size_t h1 = desc.mVertexFormat;
        std::size_t h2 = desc.mGroupSize;
        std::size_t h3 = desc.mTextureFormats.size();
        std::size_t h4 = desc.mSamples;
        std::size_t h = h1 ^ (h2 << 4) ^ (h3 << 8) ^ (h4 << 12);
        for (Engine::Render::TextureFormat format : desc.mTextureFormats)
            h = (h << 1) ^ format;
        return h;
    }
};

}

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12Pipeline : Pipeline {

        bool link(typename DirectX12VertexShaderLoader::Handle vertexShader, typename DirectX12PixelShaderLoader::Handle pixelShader);
        bool link(typename DirectX12VertexShaderLoader::Ptr vertexShader, typename DirectX12PixelShaderLoader::Ptr pixelShader);

        ID3D12PipelineState *get(VertexFormat vertexFormat, size_t groupSize, DirectX12RenderTarget *target, bool depthChecking = true) const;

        //const std::array<std::array<std::array<ReleasePtr<ID3D12PipelineState>, 3>, 3>, 3> *ptr() const;

        void reset();

        void setName(std::string_view name);

    private:
        mutable std::unordered_map<PipelineDescriptor, ReleasePtr<ID3D12PipelineState>> mPipelines;

        std::variant<typename DirectX12VertexShaderLoader::Handle, typename DirectX12VertexShaderLoader::Ptr> mVertexShader;
        std::variant<typename DirectX12PixelShaderLoader::Handle, typename DirectX12PixelShaderLoader::Ptr> mPixelShader;

        std::string mName;
    };

}
}