#include "vulkantoolslib.h"

#include "vulkanrendercontexttool.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/imageloaderlib.h"

#include "Madgine/render/fonts/fontloader.h"

#include "Madgine/imageloader/imagedata.h"

#include "Vulkan/vulkanrendercontext.h"

UNIQUECOMPONENT(Engine::Tools::VulkanRenderContextTool);

METATABLE_BEGIN_BASE(Engine::Tools::VulkanRenderContextTool, Engine::Tools::RenderContextTool)
METATABLE_END(Engine::Tools::VulkanRenderContextTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::VulkanRenderContextTool, Engine::Tools::RenderContextTool)
SERIALIZETABLE_END(Engine::Tools::VulkanRenderContextTool)

namespace Engine {
namespace Tools {

    VulkanRenderContextTool::VulkanRenderContextTool(ImRoot &root)
        : ToolVirtualImpl<VulkanRenderContextTool, RenderContextTool>(root)
    {
    }

    Threading::Task<bool> VulkanRenderContextTool::init()
    {
        mImageTexture = { Render::TextureType_2D, false, Render::FORMAT_RGBA8, { 100, 100 } };

        getTool<Inspector>().addPreviewDefinition<Resources::ImageLoader::Resource>([this](Resources::ImageLoader::Resource *image) {
            Resources::ImageLoader::Handle data = image->loadData();
            data.info()->setPersistent(true);

            mImageTexture.setData(data->mSize, data->mBuffer);
            ImGui::Image((void *)mImageTexture.handle(), data->mSize);

            return false;
        });

        co_return co_await RenderContextTool::init();
    }

    Threading::Task<void> VulkanRenderContextTool::finalize()
    {
        mImageTexture.reset();

        co_await RenderContextTool::finalize();
    }

    void VulkanRenderContextTool::renderMetrics()
    {
        if (ImGui::CollapsingHeader("Vulkan")) {
            ImGui::Text("Bytes/frame:");
            ImGui::SameLine();
            ImGui::Bytes(mTempBytesPerFrame.average());      
            float ratio = mTempBytesPerFrame.average() / Render::VulkanMappedHeapAllocator::goodSize;            
            ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value, "RingBuffer usage: %.1f%%", 100.0f * ratio);            
            ImGui::PlotHistory(mTempBytesPerFrameTrend, "Temp Bytes per Frame", ImGui::sByteUnits);
        }
    }

    void VulkanRenderContextTool::renderStatus()
    {
        float ratio = mTempBytesPerFrame.average() / Render::VulkanMappedHeapAllocator::goodSize;        
        ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value , "O %.1f%%", 100.0f * ratio);        
        ImGui::Separator();
    }

    void VulkanRenderContextTool::update()
    {
        size_t totalTempBytes = Render::VulkanRenderContext::getSingleton().tempAllocatorMemoryQuota();

        size_t frameBytes = totalTempBytes - mLastFrameTempBytes;
        mLastFrameTempBytes = totalTempBytes;

        mTempBytesPerFrame << frameBytes;

        mTimeBank += ImGui::GetIO().DeltaTime;
        if (mTimeBank >= 0.5f) {
            mTimeBank = fmodf(mTimeBank, 0.5f);
            mTempBytesPerFrameTrend << mTempBytesPerFrame.average();
        }

        ToolBase::update();
    }

    std::string_view VulkanRenderContextTool::key() const
    {
        return "VulkanRenderContextTool";
    }

}
}
