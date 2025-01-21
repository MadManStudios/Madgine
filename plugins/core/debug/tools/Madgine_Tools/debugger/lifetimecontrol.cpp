#include "../debugtoolslib.h"

#include "lifetimecontrol.h"

#include "Madgine_Tools/imguiicons.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/debug/debuggablelifetime.h"

#include "NodeEditor/imgui_node_editor.h"

#include "Madgine/trees/treeformat.h"

#include "Madgine_Tools/renderer/imroot.h"

UNIQUECOMPONENT(Engine::Tools::LifetimeControl);

METATABLE_BEGIN_BASE(Engine::Tools::LifetimeControl, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::LifetimeControl)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::LifetimeControl, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::LifetimeControl)

namespace Engine {

namespace Debug {
    DebuggableLifetimeBase &getRootLifetime();
}

namespace Tools {

    LifetimeControl::LifetimeControl(ImRoot &root)
        : Tool<LifetimeControl>(root)
    {
    }

    Threading::Task<bool> LifetimeControl::init()
    {
        mEditor = { ed::CreateEditor(), &ed::DestroyEditor };

        ed::SetCurrentEditor(mEditor.get());
        ed::Style &style = ed::GetStyle();
        style.LinkStrength = 0.0f;
        style.PivotAlignment = { 0.5f, 0.5f };

        ed::SetCurrentEditor(nullptr);

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> LifetimeControl::finalize()
    {
        mEditor.reset();

        co_await ToolBase::finalize();
    }

    void controls(Debug::DebuggableLifetimeBase &lifetime)
    {
        ImGui::BeginHorizontal("Controls");

        ImGui::Spring();

        auto pre = [](bool b) { if (b) ImGui::BeginDisabled(); };
        auto post = [](bool b) { if (b) ImGui::EndDisabled(); };

        bool b = lifetime.running();
        pre(b);
        if (ImGui::Button(IMGUI_ICON_PLAY)) {
            lifetime.startLifetime();
        }
        post(b);

        b = !lifetime.running();
        pre(b);
        if (ImGui::Button(IMGUI_ICON_PAUSE)) {
            // pause();
        }
        post(b);

        pre(b);
        if (ImGui::Button(IMGUI_ICON_STOP)) {
            lifetime.endLifetime();
        }
        post(b);

        ImGui::Text(std::to_string(lifetime.debugContexts().size()));

        ImGui::Spring();

        ImGui::EndHorizontal();
    }

    void renderLifetime(Debug::DebuggableLifetimeBase &lifetime)
    {
        ImGui::PushID(&lifetime);
        ed::BeginNode(reinterpret_cast<uintptr_t>(&lifetime));
        ImGui::BeginVertical("vert");

        ImGui::BeginHorizontal("Header");
        ImGui::Spring();
        ed::BeginPin(reinterpret_cast<uintptr_t>(&lifetime), ed::PinKind::Input);
        ImGui::Text("O");
        ed::EndPin();
        ImGui::Spring();
        ImGui::EndHorizontal();

        ImGui::BeginHorizontal("Content");
        ImGui::Text(lifetime.owner().name());
        ImGui::EndHorizontal();

        controls(lifetime);

        ImGui::BeginHorizontal("Footer");
        ImGui::Spring();
        ed::BeginPin(reinterpret_cast<uintptr_t>(&lifetime) + 1, ed::PinKind::Output);
        ImGui::Text("O");
        ed::EndPin();
        ImGui::Spring();
        ImGui::EndHorizontal();

        ImGui::EndVertical();
        ed::EndNode();
        ImGui::PopID();

        for (Debug::DebuggableLifetimeBase &child : lifetime.children()) {
            renderLifetime(child);
            ed::Link(reinterpret_cast<uintptr_t>(&child), reinterpret_cast<uintptr_t>(&lifetime) + 1, reinterpret_cast<uintptr_t>(&child));
        }
    }

    void LifetimeControl::update()
    {
        ToolBase::update();
        renderToolbar();
    }

    void LifetimeControl::render()
    {
        renderTreeView();
    }

    void LifetimeControl::renderMenu()
    {
        ToolBase::renderMenu();
    }

    std::string_view LifetimeControl::key() const
    {
        return "Lifetime Control";
    }

    void LifetimeControl::renderTreeView()
    {
        if (ImGui::Begin("Lifetime Control", &mVisible)) {

            ImVec2 oldViewportPos = ImGui::GetCurrentContext()->MouseViewport->Pos;
            ImVec2 oldViewportSize = ImGui::GetCurrentContext()->MouseViewport->Size;

            ImGui::GetCurrentContext()->MouseViewport->Pos = { -10000, -10000 };
            ImGui::GetCurrentContext()->MouseViewport->Size = { 20000, 20000 };

            ed::SetCurrentEditor(mEditor.get());

            ed::Begin("Node editor");

            Debug::DebuggableLifetimeBase &root = Debug::getRootLifetime();

            for (Debug::DebuggableLifetimeBase &child : root.children())
                renderLifetime(child);

            ed::Suspend();

            if (ed::ShowBackgroundContextMenu()) {
                ImGui::OpenPopup("Context");
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            if (ImGui::BeginPopup("Context")) {
                if (ImGui::MenuItem("Format")) {
                    FormatTree format { root, &Debug::DebuggableLifetimeBase::children };
                    format.format();
                    format.visit([](Debug::DebuggableLifetimeBase *lifetime, float x, float y) {
                        ImVec2 size = ed::GetNodeSize(reinterpret_cast<uintptr_t>(lifetime));
                        ed::SetNodePosition(reinterpret_cast<uintptr_t>(lifetime), { 270.0f * x - 0.5f * size.x, 100.0f * y - 0.5f * size.y });
                    });
                    ed::NavigateToContent();
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

            ed::Resume();

            ed::End();

            ed::SetCurrentEditor(nullptr);

            ImGui::GetCurrentContext()->MouseViewport->Pos = oldViewportPos;
            ImGui::GetCurrentContext()->MouseViewport->Size = oldViewportSize;
        }
        ImGui::End();
    }

    void LifetimeControl::renderToolbar()
    {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);
        if (ImGui::Begin("Lifetime Control - Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Up, 0.01f, true, ImGuiCond_FirstUseEver);

            controls(Debug::getRootLifetime());
        }
        ImGui::End();
    }

}
}
