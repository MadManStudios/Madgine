#include "rendertoolslib.h"

#include "interactivecamera.h"

#include "imgui/imgui.h"

#include "Madgine/render/camera.h"

#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

	
    void InteractiveCamera(const ImGui::InteractiveViewState &state, Render::Camera &camera)
    {
        ImGuiIO &io = ImGui::GetIO();

        if (state.mActive) {
            camera.mPosition += camera.mOrientation * Vector3 { Vector3::UNIT_Z } * io.MouseWheel / 5.0f;
        }

        if (state.mDragging[2]) {
            camera.mPosition += camera.mOrientation * Vector3 { -io.MouseDelta.x / 50.0f, io.MouseDelta.y / 50.0f, 0.0f };
        }

        if (state.mDragging[1]) {
            camera.mOrientation = Quaternion { io.MouseDelta.x / 200.0f, Vector3::UNIT_Y } * camera.mOrientation * Quaternion { io.MouseDelta.y / 200.0f, Vector3::UNIT_X };
        }
    }

}
}