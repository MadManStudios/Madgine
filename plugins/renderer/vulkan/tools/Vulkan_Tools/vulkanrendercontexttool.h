#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Vulkan/util/vulkantexture.h"
#include "Madgine_Tools/render/rendercontexttool.h"

#include "Modules/debug/history.h"

namespace Engine {
namespace Tools {

    struct VulkanRenderContextTool : public ToolVirtualImpl<VulkanRenderContextTool, RenderContextTool> {

        SERIALIZABLEUNIT(VulkanRenderContextTool)

        VulkanRenderContextTool(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void renderMetrics() override;
        virtual void renderStatus() override;

        virtual void update() override;

        std::string_view key() const override;

    private:
        Render::VulkanTexture mImageTexture;
        size_t mLastFrameTempBytes = 0;
        float mTimeBank = 0.0f;

        Debug::History<float, 120> mTempBytesPerFrame;

        Debug::History<float, 100> mTempBytesPerFrameTrend;
    };

}
}

REGISTER_TYPE(Engine::Tools::VulkanRenderContextTool)