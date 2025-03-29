#include "opengltoolslib.h"

#include "openglrendercontexttool.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Generic/bytebuffer.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "imgui/imgui.h"

#include "Madgine/imageloaderlib.h"
#include "Madgine/imageloader/imageloader.h"

#include "Madgine/render/fonts/fontloader.h"

#include "Madgine/imageloader/imagedata.h"

UNIQUECOMPONENT(Engine::Tools::OpenGLRenderContextTool);

METATABLE_BEGIN_BASE(Engine::Tools::OpenGLRenderContextTool, Engine::Tools::RenderContextTool)
METATABLE_END(Engine::Tools::OpenGLRenderContextTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::OpenGLRenderContextTool, Engine::Tools::RenderContextTool)
SERIALIZETABLE_END(Engine::Tools::OpenGLRenderContextTool)

namespace Engine {
namespace Tools {

    OpenGLRenderContextTool::OpenGLRenderContextTool(ImRoot &root)
        : ToolVirtualImpl<OpenGLRenderContextTool, RenderContextTool>(root)
    {
    }

    Threading::Task<bool> OpenGLRenderContextTool::init()
    {
        mImageTexture = { Render::TextureType_2D, Render::FORMAT_RGBA8_SRGB };

        getTool<Inspector>().addPreviewDefinition<Resources::ImageLoader::Resource>([this](Resources::ImageLoader::Resource *image) {
            Resources::ImageLoader::Handle data = image->loadData();
            data.info()->setPersistent(true);

            if (data.available()) {
                mImageTexture.setData(data->mSize, data->mBuffer);
                ImGui::Image((void *)(uintptr_t)mImageTexture.resourceBlock(), data->mSize);
            }
            return false;
        });

        co_return co_await RenderContextTool::init();
    }

    Threading::Task<void> OpenGLRenderContextTool::finalize()
    {
        mImageTexture.reset();

        co_await RenderContextTool::finalize();
    }

    std::string_view OpenGLRenderContextTool::key() const
    {
        return "OpenGLRenderContextTool";
    }

}
}
