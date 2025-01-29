#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_RENDER_TOOLS_EXPORT RenderContextTool : public ToolVirtualBase<RenderContextTool> {

        SERIALIZABLEUNIT(RenderContextTool)

        RenderContextTool(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void render() override;
        virtual void update() override;

        std::string_view key() const override;

        bool mRenderDebugVisualizations = false;

    protected:
        void debugDraw(const Render::RenderTarget *target);
        void debugDraw(const Render::RenderPass *pass, float aspectRatio);

        void debugDrawImpl(const Render::RenderDebuggable *debuggable, float aspectRatio);

    private:
        Render::RenderContext *mContext = nullptr;
    };

}
}

REGISTER_TYPE(Engine::Tools::RenderContextTool)