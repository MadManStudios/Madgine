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

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void update() override;
        void renderMetrics() override;        

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