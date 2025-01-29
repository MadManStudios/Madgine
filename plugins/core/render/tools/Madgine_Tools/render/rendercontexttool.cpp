#include "../rendertoolslib.h"

#include "rendercontexttool.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "im3d/im3d.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Madgine/fontloader/fontloader.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/renderpass.h"
#include "Madgine/render/rendertarget.h"

VIRTUALUNIQUECOMPONENTBASE(Engine::Tools::RenderContextTool);

METATABLE_BEGIN_BASE(Engine::Tools::RenderContextTool, Engine::Tools::ToolBase)
MEMBER(mRenderDebugVisualizations)
METATABLE_END(Engine::Tools::RenderContextTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::RenderContextTool, Engine::Tools::ToolBase)
FIELD(mRenderDebugVisualizations)
SERIALIZETABLE_END(Engine::Tools::RenderContextTool)

namespace Engine {
namespace Tools {

    RenderContextTool::RenderContextTool(ImRoot &root)
        : ToolVirtualBase<RenderContextTool>(root)
    {
    }

    Threading::Task<bool> RenderContextTool::init()
    {
        mContext = &Render::RenderContext::getSingleton();

        getTool<Inspector>().addPreviewDefinition<Render::FontLoader::Resource>([](Render::FontLoader::Resource *font) {
            Render::FontLoader::Handle handle = font->loadData();
            handle.info()->setPersistent(true);
            ImGui::Image((void *)handle->mTexture->handle(), handle->mTexture->size());
            return false;
        });

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> RenderContextTool::finalize()
    {
        co_await ToolBase::finalize();
    }

    void RenderContextTool::render()
    {
        if (ImGui::Begin("RenderContext - Tool", &mVisible)) {
            ImGui::Checkbox("Show Debug Visualizers", &mRenderDebugVisualizations);
        }
        ImGui::End();
    }

    void RenderContextTool::update()
    {
        ToolBase::update();

        if (mRenderDebugVisualizations) {
            for (const Render::RenderTarget *target : mContext->renderTargets()) {
                debugDraw(target);
            }
        }
    }

    std::string_view RenderContextTool::key() const
    {
        return "RenderContextTool";
    }

    void RenderContextTool::debugDraw(const Render::RenderTarget *target)
    {
        if (target->blitSource())
            debugDraw(target->blitSource());

        Vector2i size = target->size();

        for (const Render::RenderPass *pass : target->renderPasses()) {
            debugDraw(pass, static_cast<float>(size.x) / size.y);
        }
    }

    void RenderContextTool::debugDraw(const Render::RenderPass *pass, float aspectRatio)
    {
        debugDrawImpl(pass, aspectRatio);

        for (const Render::RenderData *dependency : pass->dependencies()) {
            const Render::RenderTarget *target = dynamic_cast<const Render::RenderTarget *>(dependency);
            if (target)
                debugDraw(target);
        }
    }

    void RenderContextTool::debugDrawImpl(const Render::RenderDebuggable *debuggable, float aspectRatio)
    {
        debuggable->debugFrustums([](const Frustum &frustum, std::string_view name) {
            Im3D::Frustum(frustum);
            if (Im3D::IsObjectHovered()) {
                ImGui::Text(name);
            }
        });
        debuggable->debugCameras([=](const Render::Camera &camera, std::string_view name) {
            Im3D::Frustum(camera.getFrustum(aspectRatio), { .mColor = { 0.0f, 1.0f, 0.0f, 1.0f } });
        });
    }

}
}
