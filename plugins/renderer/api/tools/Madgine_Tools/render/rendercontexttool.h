#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_RENDER_TOOLS_EXPORT RenderContextTool : public ToolVirtualBase<RenderContextTool> {

        SERIALIZABLEUNIT(RenderContextTool)

        RenderContextTool(ImRoot &root);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void renderSettings() override;
        void update() override;
        void renderMenu() override;

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