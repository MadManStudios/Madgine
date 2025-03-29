#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "DirectX12/util/directx12texture.h"
#include "Madgine_Tools/render/rendercontexttool.h"

namespace Engine {
namespace Tools {

    struct DirectX12RenderContextTool : public ToolVirtualImpl<DirectX12RenderContextTool, RenderContextTool> {

        SERIALIZABLEUNIT(DirectX12RenderContextTool)

        DirectX12RenderContextTool(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        std::string_view key() const override;

    private:
        Render::DirectX12Texture mImageTexture;
    };

}
}

REGISTER_TYPE(Engine::Tools::DirectX12RenderContextTool)