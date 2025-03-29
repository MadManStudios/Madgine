#include "../directx12lib.h"

#include "directx12pipeline.h"

#include "../directx12rendercontext.h"

#include "../directx12rendertarget.h"

namespace Engine {
namespace Render {

    bool DirectX12Pipeline::link(typename DirectX12VertexShaderLoader::Handle vertexShader, typename DirectX12PixelShaderLoader::Handle pixelShader)
    {
        reset();

        mVertexShader = std::move(vertexShader);
        mPixelShader = std::move(pixelShader);

        return true;
    }

    bool DirectX12Pipeline::link(typename DirectX12VertexShaderLoader::Ptr vertexShader, typename DirectX12PixelShaderLoader::Ptr pixelShader)
    {
        reset();

        mVertexShader = std::move(vertexShader);
        mPixelShader = std::move(pixelShader);

        return true;
    }

    ID3D12PipelineState *DirectX12Pipeline::get(VertexFormat vertexFormat, size_t groupSize, DirectX12RenderTarget *target, bool depthChecking) const
    {
        size_t samplesBits = sqrt(target->samples());
        assert(samplesBits * samplesBits == target->samples());

        std::vector<TextureFormat> formats;
        for (size_t i = 0; i < target->textureCount(); ++i)
            formats.emplace_back(target->textureFormat(i));
        PipelineDescriptor desc {
            vertexFormat,
            groupSize,
            formats,
            target->samples()
        };
        ReleasePtr<ID3D12PipelineState> &pipeline = mPipelines[desc];

        if (!pipeline) {

            auto checkVertex = [](auto &variant) {
                return std::visit(overloaded {
                                      [](const typename DirectX12VertexShaderLoader::Handle &handle) {
                                          return !handle || handle.available();
                                      },
                                      [](const typename DirectX12VertexShaderLoader::Ptr &ptr) {
                                          return true;
                                      } },
                    variant);
            };
            auto checkPixel = [](auto &variant) {
                return std::visit(overloaded {
                                      [](const typename DirectX12PixelShaderLoader::Handle &handle) {
                                          return !handle || handle.available();
                                      },
                                      [](const typename DirectX12PixelShaderLoader::Ptr &ptr) {
                                          return true;
                                      } },
                    variant);
            };

            auto resolve = [](auto &variant) -> IDxcBlob * {
                return std::visit(
                    [](const auto &handle) -> IDxcBlob * {
                        return handle ? *handle : nullptr;
                    },
                    variant);
            };

            if (!checkVertex(mVertexShader))
                return nullptr;
            if (!checkPixel(mPixelShader))
                return nullptr;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
            ZeroMemory(&pipelineDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

            pipelineDesc.VS = { resolve(mVertexShader)->GetBufferPointer(), resolve(mVertexShader)->GetBufferSize() };
            if (resolve(mPixelShader))
                pipelineDesc.PS = { resolve(mPixelShader)->GetBufferPointer(), resolve(mPixelShader)->GetBufferSize() };

            std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayoutDesc = DirectX12RenderContext::createVertexLayout(vertexFormat);

            pipelineDesc.InputLayout.pInputElementDescs = vertexLayoutDesc.data();
            pipelineDesc.InputLayout.NumElements = vertexLayoutDesc.size();

            pipelineDesc.pRootSignature = DirectX12RenderContext::getSingleton().mRootSignature;

            pipelineDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            pipelineDesc.RasterizerState.CullMode = /*D3D12_CULL_BACK*/ D3D12_CULL_MODE_NONE;
            pipelineDesc.RasterizerState.DepthBias = 0;
            pipelineDesc.RasterizerState.DepthBiasClamp = 0.0f;
            pipelineDesc.RasterizerState.DepthClipEnable = FALSE;
            pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            pipelineDesc.RasterizerState.FrontCounterClockwise = FALSE;
            pipelineDesc.RasterizerState.MultisampleEnable = FALSE;
            pipelineDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
            pipelineDesc.RasterizerState.ForcedSampleCount = 0;
            pipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

            pipelineDesc.BlendState.AlphaToCoverageEnable = FALSE;
            pipelineDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
            pipelineDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            pipelineDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            pipelineDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            pipelineDesc.DepthStencilState.DepthEnable = depthChecking;
            pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            pipelineDesc.DepthStencilState.StencilEnable = FALSE;
            pipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

            constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE types[] {
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
            };

            pipelineDesc.PrimitiveTopologyType = types[groupSize - 1];

            pipelineDesc.NodeMask = 1;

            pipelineDesc.SampleMask = UINT_MAX;

            pipelineDesc.NumRenderTargets = target->textureCount();
            for (size_t i = 0; i < target->textureCount(); ++i) {
                switch (target->textureFormat(i)) {
                case FORMAT_RGBA8:
                    pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
                    break;
                case FORMAT_RGBA16F:
                    pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    break;
                case FORMAT_RGBA8_SRGB:
                    pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                    break;
                default:
                    std::terminate();
                }
            }

            pipelineDesc.SampleDesc.Count = target->samples();
            pipelineDesc.SampleDesc.Quality = 0;

            HRESULT hr = GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline));
            DX12_CHECK(hr);

            pipeline->SetName(StringUtil::toWString(mName).c_str());
        }

        return pipeline;
    }

    void DirectX12Pipeline::reset()
    {
        mPipelines.clear();
        mVertexShader = {};
        mPixelShader = {};
    }

    void DirectX12Pipeline::setName(std::string_view name)
    {
        mName = name;
    }

}
}