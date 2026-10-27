#include "Madgine/imageloaderlib.h"
#include "directx12toolslib.h"

#include "directx12rendercontexttool.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/fonts/fontloader.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "DirectX12/directx12rendercontext.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#if ENABLE_TASK_TRACKING
#    include "Madgine_Tools/tasktracker/tasktracker.h"
#endif

UNIQUECOMPONENT(Engine::Tools::DirectX12RenderContextTool);

METATABLE_BEGIN_BASE(Engine::Tools::DirectX12RenderContextTool, Engine::Tools::RenderContextTool)
METATABLE_END(Engine::Tools::DirectX12RenderContextTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::DirectX12RenderContextTool, Engine::Tools::RenderContextTool)
SERIALIZETABLE_END(Engine::Tools::DirectX12RenderContextTool)

namespace Engine {
namespace Tools {

    DirectX12RenderContextTool::DirectX12RenderContextTool(ImRoot &root)
        : ToolVirtualImpl<DirectX12RenderContextTool, RenderContextTool>(root)
    {
    }

    Threading::Task<bool> DirectX12RenderContextTool::init()
    {

        getTool<Inspector>().addPreviewDefinition<Resources::ImageLoader::Resource>([this](const Traced<Resources::ImageLoader::Resource *> &image) {
            mRoot.Image(image.get()->path());
            return false;
        });

#if ENABLE_TASK_TRACKING
        getTool<TaskTracker>().registerCustomTracker("Graphics Queue", Render::DirectX12RenderContext::getSingleton().mGraphicsQueue.mTracker);
#endif

        co_return co_await RenderContextTool::init();
    }

    Threading::Task<void> DirectX12RenderContextTool::finalize()
    {
        co_await RenderContextTool::finalize();
    }

    void DirectX12RenderContextTool::renderMetrics()
    {
        if (ImGui::CollapsingHeader("DirectX12")) {
            ImGui::Text("Temp Bytes/frame:");
            ImGui::SameLine();
            ImGui::Bytes(mTempBytesPerFrame.average());
            ImGui::Text("Upload Bytes/frame:");
            ImGui::SameLine();
            ImGui::Bytes(mUploadBytesPerFrame.average());
            float ratio = mTempBytesPerFrame.average() / Render::DirectX12MappedHeapAllocator::goodSize;
            ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value, "RingBuffer usage: %.1f%%", 100.0f * ratio);
            ImGui::PlotHistory(mTempBytesPerFrameTrend, "Temp Bytes per Frame", ImGui::sByteUnits);
            ratio = mUploadBytesPerFrame.average() / Render::DirectX12MappedHeapAllocator::goodSize;
            ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value, "Upload usage: %.1f%%", 100.0f * ratio);
            ImGui::PlotHistory(mUploadBytesPerFrameTrend, "Temp Bytes per Frame", ImGui::sByteUnits);
        }
    }

    void DirectX12RenderContextTool::update()
    {
        size_t totalTempBytes = Render::DirectX12RenderContext::getSingleton().tempAllocatorMemoryQuota();
        auto totalUploadBytes = Render::DirectX12RenderContext::getSingleton().uploadAllocatorMemoryQuota();

        size_t frameTempBytes = totalTempBytes - mLastFrameTempBytes;
        mLastFrameTempBytes = totalTempBytes;

        size_t frameUploadBytes = TupleUnpacker::accumulate(totalUploadBytes, [](size_t val, size_t acc) { return std::max(val, acc); }, 0);

        mTempBytesPerFrame << frameTempBytes;
        mUploadBytesPerFrame << frameUploadBytes;

        mTimeBank += ImGui::GetIO().DeltaTime;
        if (mTimeBank >= 0.5f) {
            mTimeBank = fmodf(mTimeBank, 0.5f);
            mTempBytesPerFrameTrend << mTempBytesPerFrame.average();
            mUploadBytesPerFrameTrend << mUploadBytesPerFrame.average();
        }

        if (ImGui::BeginStatus()) {
            float ratio = mTempBytesPerFrame.average() / Render::DirectX12MappedHeapAllocator::goodSize;
            ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value, "O %.1f%%", 100.0f * ratio);
            ratio = mUploadBytesPerFrame.average() / Render::DirectX12MappedHeapAllocator::goodSize;
            ImGui::TextColored(ImColor::HSV((1.0f - ratio) / 3.0f, 1.0f, 1.0f).Value, "U %.1f%%", 100.0f * ratio);
            ImGui::EndStatus();
        }

        ToolBase::update();
    }

    std::string_view DirectX12RenderContextTool::key() const
    {
        return "DirectX12RenderContextTool";
    }

}
}
