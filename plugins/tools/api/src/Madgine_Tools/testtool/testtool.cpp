#include "../toolslib.h"

#include "testtool.h"

#include "../renderer/imroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/window/windowapi.h"

#include "Meta/math/atlas2.h"

#include "im3d/im3d.h"

#include "Madgine/render/vertex.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

UNIQUECOMPONENT(Engine::Tools::TestTool);

METATABLE_BEGIN_BASE(Engine::Tools::TestTool, Engine::Tools::ToolBase)
FUNCTION(logTest)
FUNCTION(logValue, value)
FUNCTION(dummy)
MEMBER(mV)
MEMBER(mFlags)
METATABLE_END(Engine::Tools::TestTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::TestTool, Engine::Tools::ToolBase)
FIELD(mFlags)
SERIALIZETABLE_END(Engine::Tools::TestTool)

namespace Engine {
namespace Tools {

    TestTool::TestTool(ImRoot &root)
        : Tool<TestTool>(root)
    {
    }

    template <size_t... Is>
    void renderValuetypeSizes(std::index_sequence<Is...>)
    {
        std::vector<std::pair<const char *, unsigned int>> data = {
            { typeid(std::get<Is>(std::declval<ValueType::Union>())).name(), sizeof(std::get<Is>(std::declval<ValueType::Union>())) }...
        };
        for (auto [name, size] : data) {
            ImGui::Text("  %s size: %u", name, size);
        }
    }

    void TestTool::render()
    {
        if (ImGui::Begin("TestTool", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Right, 0.2f, false, ImGuiCond_FirstUseEver);

            if (ImGui::CollapsingHeader("ImGui")) {
                ImGui::DragFloat2("Scale", &ImGui::GetIO().DisplayFramebufferScale.x, 0.1f, 0.1f, 2.0f);
            }
            if (ImGui::CollapsingHeader("Im3D")) {

                ImGui::Checkbox("3D Text", &mRenderText);
                ImGui::SameLine();
                ImGui::InputText("##Text", &m3DText);

                if (mRenderText)
                    Im3D::Text(m3DText.c_str(), {});

                ImGui::Checkbox("Sphere", &mRenderSphere);
                ImGui::SameLine();
                ImGui::DragInt("Detail", &mSphereDetail, 1.0f, 0, 100);

                if (mRenderSphere)
                    Im3D::Sphere({ 0, 0, 0 }, 1.0f, { .mDetail = (size_t)mSphereDetail });

                ImGui::Checkbox("3D Arrow", &mRenderArrow);

                if (mRenderArrow)
                    Im3D::Arrow(0.15f, Vector3::ZERO, Vector3::UNIT_X);
            }
            if (ImGui::CollapsingHeader("Misc")) {
                if (ImGui::TreeNode("ValueType Sizes")) {
                    ImGui::Text("ValueType size: %u", (unsigned int)sizeof(ValueType));
                    renderValuetypeSizes(std::make_index_sequence<size_t(ValueTypeEnum::MAX_VALUETYPE_TYPE)>());
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }

    void TestTool::update()
    {

        ToolBase::update();
    }

    std::string_view TestTool::key() const
    {
        return "TestTool";
    }

    void TestTool::logTest()
    {
        LOG("Test");
    }

    void TestTool::logValue(const ValueType &v)
    {
        LOG(v);
    }

    int TestTool::dummy()
    {
        return 42;
    }

}
}
