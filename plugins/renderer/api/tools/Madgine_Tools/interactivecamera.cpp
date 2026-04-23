#include "rendertoolslib.h"

#include "interactivecamera.h"

#include "Madgine/render/camera.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

    void InteractiveCamera(ImGui::InteractiveViewResultFlags flags, Render::Camera &camera)
    {
        ImGuiIO &io = ImGui::GetIO();

        if (flags & (ImGui::InteractiveViewResultFlags_Active | ImGui::InteractiveViewResultFlags_Hovered)) {
            camera.mPosition += camera.mOrientation * Vector3 { Vector3::UNIT_Z } * io.MouseWheel / 5.0f;
        }

        if (flags & ImGui::InteractiveViewResultFlags_Active) {
            int mouseButton = (flags & ImGui::InteractiveViewResultFlags_MouseButtonMask_) - 1;
            if (mouseButton == 2) {
                camera.mPosition += camera.mOrientation * Vector3 { -io.MouseDelta.x / 50.0f, io.MouseDelta.y / 50.0f, 0.0f };
            }

            if (mouseButton == 1) {
                camera.mOrientation = Quaternion { io.MouseDelta.x / 200.0f, Vector3::UNIT_Y } * camera.mOrientation * Quaternion { io.MouseDelta.y / 200.0f, Vector3::UNIT_X };
            }
        }
    }

}
}