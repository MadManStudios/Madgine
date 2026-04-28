#include "../../scenerenderertoolslib.h"

#include "sceneeditortests.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/tests/tooltests.h"
#include "imgui_test_engine/imgui_test_engine/imgui_te_context.h"

namespace Engine {
namespace Tools {

    void registerSceneEditorTests(ImGuiTestEngine *e)
    {
        ImGuiTest *t = IM_REGISTER_TEST(e, "Usage", "New Scene");
        t->TestFunc = [](ImGuiTestContext *ctx) {
            ctx->WindowFocus("Game");

            ctx->SetRef("##MainMenuBar");
            ctx->MenuClick("Resources/New.../Scene");
                        
            ctx->Yield(2);

            SetToolWindowRef(ctx, "SceneView1", "<unnamed>", "Scene");
            
            ctx->KeyPress(ImGuiMod_Ctrl | ImGuiKey_N, 1);

            auto window = SetToolWindowRef(ctx, "Details", "<unnamed>", "Scene");
            IM_CHECK_NE(window, nullptr);

            ctx->MouseMoveToPos(window->Rect().GetCenter());
            ctx->MouseClick(1);
            ctx->SetRef("//$FOCUSED");
            ctx->MenuClick(IMGUI_ICON_PLUS " Add Component/Transform");

            ctx->MouseMoveToPos(window->Rect().GetCenter());
            ctx->MouseClick(1);
            ctx->SetRef("//$FOCUSED");
            ctx->MenuClick(IMGUI_ICON_PLUS " Add Component/Mesh");

            ctx->SetRef(window);
            ctx->ComboClick("columnsmMesh##suggestions/Cube");

            ctx->MouseMoveToPos(window->Rect().GetCenter());
            ctx->MouseClick(1);
            ctx->SetRef("//$FOCUSED");
            ctx->MenuClick(IMGUI_ICON_PLUS " Add Behavior/Native/Rotate");
            
            /* auto window = SetContentRef(ctx, "<unnamed>", "SceneEditor");
            IM_CHECK_NE(window, nullptr);

            ctx->MouseMoveToPos(window->Rect().GetCenter());
            ctx->MouseClick(1);
            ctx->SetRef("//$FOCUSED");*/
            //ctx->MenuClick(IMGUI_ICON_PLUS " Add Node/Native/Rotate");
        };
    }

}
}
