#include "../../nodegraphtoolslib.h"

#include "nodegrapheditortests.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/tests/tooltests.h"
#include "imgui_test_engine/imgui_test_engine/imgui_te_context.h"

namespace Engine {
namespace Tools {

    void registerNodeGraphEditorTests(ImGuiTestEngine *e)
    {
        ImGuiTest *t = IM_REGISTER_TEST(e, "Usage", "New NodeGraph");
        t->TestFunc = [](ImGuiTestContext *ctx) {
            ctx->WindowFocus("Game");

            ctx->SetRef("##MainMenuBar");
            ctx->MenuClick("Resources/New.../Node Graph");

            auto window = SetToolWindowRef(ctx, "Content", "<unnamed>", "NodeGraphEditor");
            IM_CHECK_NE(window, nullptr);

            ctx->MouseMoveToPos(window->Rect().GetCenter());
            ctx->MouseClick(1);
            ctx->SetRef("//$FOCUSED");
            ctx->MenuClick(IMGUI_ICON_PLUS " Add Node/Native/Rotate");
        };
    }

}
}
