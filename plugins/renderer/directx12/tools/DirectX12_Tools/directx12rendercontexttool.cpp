#include "Madgine/imageloaderlib.h"
#include "directx12toolslib.h"

#include "directx12rendercontexttool.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/fonts/fontloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "DirectX12/directx12rendercontext.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "imgui/imgui.h"

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
        getTool<TaskTracker>().registerCustomTracker("Graphics Queue", &static_cast<Render::DirectX12RenderContext *>(static_cast<ClientImRoot &>(mRoot).window().getRenderer())->mGraphicsQueue.mTracker);
#endif

        co_return co_await RenderContextTool::init();
    }

    Threading::Task<void> DirectX12RenderContextTool::finalize()
    {
        co_await RenderContextTool::finalize();
    }

    std::string_view DirectX12RenderContextTool::key() const
    {
        return "DirectX12RenderContextTool";
    }

}
}
