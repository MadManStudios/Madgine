#include "../clienttoolslib.h"

#include "gamepadviewer.h"

#include "Platform/input/inputevents.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/texture.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../imgui/clientimroot.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::GamepadViewer, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::GamepadViewer)

METATABLE_BEGIN_BASE(Engine::Tools::GamepadViewer, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::GamepadViewer)

UNIQUECOMPONENT(Engine::Tools::GamepadViewer)

namespace Engine {
namespace Tools {

    GamepadViewer::GamepadViewer(ImRoot &root)
        : VirtualUnit(root)
    {
    }

    std::string_view GamepadViewer::key() const
    {
        return "GamepadViewer";
    }

    Threading::Task<bool> GamepadViewer::init()
    {
        if (!co_await ToolBase::init())
            co_return false;

        mImage.load("Gamepad");

        co_return true;
    }

    Threading::Task<void> GamepadViewer::finalize()
    {
        mGamepadTexture.reset();

        co_await ToolBase::finalize();
    }

    void GamepadViewer::render()
    {
        if (beginToolPanel("Gamepad", &mVisible, ImGuiDir_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

            ImDrawList *DrawList = ImGui::GetWindowDrawList();
            ImGuiWindow *window = ImGui::GetCurrentWindow();

            ClientImRoot &root = static_cast<ClientImRoot &>(mRoot);

            // prepare canvas
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            constexpr const float ratio = 1350.0f / 759.0f;
            const float dimX = avail.x;
            const float dimY = avail.y * ratio;
            const float dim = Math::max(20.0f, Math::min(dimX, dimY));
            ImVec2 Canvas { dim, dim / ratio };

            const float size = dim / 12.0f;
            ImVec2 Circle { size, size };

            ImVec2 cursor = window->DC.CursorPos;

            if (!mGamepadTexture && mImage.available()) {
                mGamepadTexture = Render::RenderContext::getSingleton().createTexture(
                    Render::TextureType_2D,
                    Render::FORMAT_RGBA8_SRGB,
                    mImage->mSize,
                    mImage->mBuffer);
            }

            if (mGamepadTexture)
                mRoot.Image(mGamepadTexture, Math::Vector2i { static_cast<int>(Canvas.x), static_cast<int>(Canvas.y) });

            size_t i = 0;
            constexpr float offsets[2][2] = {
                { 0.275f, 0.125f },
                { 0.553f, 0.235f }
            };

            for (Math::Vector2 axis : { root.mLeftControllerStick, root.mRightControllerStick }) {

                ImVec2 topLeft = cursor + ImVec2 { offsets[i][0] * dim, offsets[i][1] * dim };
                ++i;
                ImRect bb(topLeft, topLeft + Circle);
                ImGui::ItemSize(bb);
                if (ImGui::ItemAdd(bb, 0)) {

                    // background grid
                    for (float i = 0; i <= Circle.x; i += (Circle.x / 4)) {
                        DrawList->AddLine(
                            ImVec2(bb.Min.x + i, bb.Min.y),
                            ImVec2(bb.Min.x + i, bb.Max.y),
                            ImGui::GetColorU32(ImGuiCol_TextDisabled));
                    }
                    for (float i = 0; i <= Circle.y; i += (Circle.y / 4)) {
                        DrawList->AddLine(
                            ImVec2(bb.Min.x, bb.Min.y + i),
                            ImVec2(bb.Max.x, bb.Min.y + i),
                            ImGui::GetColorU32(ImGuiCol_TextDisabled));
                    }

                    DrawList->AddCircle(bb.Min + Circle / 2, 0.5f * size, ImGui::GetColorU32(ImGuiCol_TextDisabled), 36);

                    ImVec2 v { 0.4f * axis.x + 0.5f, 0.4f * axis.y + 0.5f };

                    ImVec2 pos = bb.Min + v * Circle;

                    DrawList->AddLine(pos - ImVec2(5.0f, 0), pos + ImVec2(5.0f, 0), ImGui::GetColorU32(ImGuiCol_ButtonHovered), 2.0f);
                    DrawList->AddLine(pos - ImVec2(0, 5.0f), pos + ImVec2(0, 5.0f), ImGui::GetColorU32(ImGuiCol_ButtonHovered), 2.0f);

                    // restore cursor pos
                    ImGui::SetCursorScreenPos(ImVec2(bb.Max.x + 20, bb.Min.y)); // :P
                }
            }

            auto renderCircle = [&](ImVec2 offset, float radius) {
                DrawList->AddCircleFilled(cursor + offset * dim, radius * dim, ImGui::GetColorU32(ImGuiCol_ButtonActive));
            };

            ImVec2 DPadOffset { 0.405f, 0.285f };
            int x = 2;
            int y = 2;
            if (root.mDPadState > 0) {
                x = root.mDPadState > 3 && root.mDPadState < 7 ? 7 - root.mDPadState : (root.mDPadState + 2) % 8 - 1;
                y = root.mDPadState > 5 ? 9 - root.mDPadState : root.mDPadState - 1;
            }
            float x_off = sqrtf(fabs(x - 2) / 2) * Math::sign(x - 2);
            float y_off = sqrtf(fabs(y - 2) / 2) * Math::sign(y - 2);
            renderCircle(DPadOffset + ImVec2 { x_off, y_off } * 0.038f, 0.013f);

            auto button = [&](Platform::Input::Key::Key key, ImVec2 off, float radius) {
                if (ImGui::IsKeyDown(static_cast<ImGuiKey>(key)))
                    renderCircle(off, radius);
            };
            button(Platform::Input::Key::GP_A, { 0.68f, 0.215f }, 0.02f);
            button(Platform::Input::Key::GP_B, { 0.73f, 0.165f }, 0.02f);
            button(Platform::Input::Key::GP_X, { 0.63f, 0.165f }, 0.02f);
            button(Platform::Input::Key::GP_Y, { 0.68f, 0.12f }, 0.02f);
            button(Platform::Input::Key::GP_LB, { 0.3f, 0.06f }, 0.02f);
            button(Platform::Input::Key::GP_RB, { 0.7f, 0.06f }, 0.02f);
            button(Platform::Input::Key::GP_LSB, { 0.315f, 0.165f }, 0.02f);
            button(Platform::Input::Key::GP_RSB, { 0.595f, 0.275f }, 0.02f);
            button(Platform::Input::Key::GP_B1, { 0.45f, 0.166f }, 0.01f);
            button(Platform::Input::Key::GP_B2, { 0.55f, 0.166f }, 0.01f);
        }
        ImGui::End();
    }

}
}