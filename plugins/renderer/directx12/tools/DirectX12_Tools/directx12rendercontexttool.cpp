#include "directx12toolslib.h"

#include "directx12rendercontexttool.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/inspector/inspector.h"

#if ENABLE_TASK_TRACKING
#include "Madgine_Tools/tasktracker/tasktracker.h"
#endif

#include "DirectX12/directx12rendercontext.h"

#include "imgui/imgui.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/imageloaderlib.h"

#include "Madgine/render/fonts/fontloader.h"

#include "Madgine/imageloader/imagedata.h"

#include "Madgine/render/textureloader.h"

#include "Madgine_Tools/renderer/imroot.h"

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

        getTool<Inspector>().addPreviewDefinition<Resources::ImageLoader::Resource>([this](Resources::ImageLoader::Resource *image) {
            mRoot.Image(image->path());
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
