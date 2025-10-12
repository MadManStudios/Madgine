#include "../widgetstoolslib.h"

#include "widgeteditor.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "imgui/imgui.h"

#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/math/bounds.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/widgets/widget.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/serialize/streams/serializestream.h"

#include "Madgine/widgets/widgetmanager.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Generic/coroutines/generator.h"

#include "Madgine_Tools/imguiicons.h"

#include "Madgine/widgets/widgetloader.h"

#include "Madgine/render/rendertarget.h"

#include "imgui/imgui_internal.h"

UNIQUECOMPONENT(Engine::Tools::WidgetEditor);

METATABLE_BEGIN_BASE(Engine::Tools::WidgetEditor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::WidgetEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::WidgetEditor, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::WidgetEditor)

namespace Engine {
namespace Tools {

    WidgetEditor::WidgetEditor(ImRoot &root)
        : Tool<WidgetEditor, ResourceEditor>(root)
    {
    }

    Threading::Task<bool> WidgetEditor::init()
    {
        mWidgetManager = &static_cast<const ClientImRoot &>(mRoot).window().getWindowComponent<Widgets::WidgetManager>();

        co_return co_await ResourceEditor::init(Widgets::WidgetLoader::getSingleton(), "Widget");
    }

    Threading::Task<void> WidgetEditor::finalize()
    {
        mFiles.clear();

        co_await ResourceEditor::finalize();
        co_return;
    }

    void WidgetEditor::renderMenu()
    {
        ResourceEditor::renderMenu();
        if (mVisible) {
        }
    }

    Widgets::WidgetManager &WidgetEditor::manager()
    {
        return *mWidgetManager;
    }

    void WidgetEditor::render()
    {
        std::erase_if(mFiles, [&, this](std::pair<Widgets::WidgetLoader::Resource *const, WidgetFile> &p) {
            return !p.second.render();
        });

        ResourceEditor::render();

        if (ImGui::Begin("Game")) {

            handleManagerInteractions(*mWidgetManager, mWidgetManager->getClientSpace().mTopLeft);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("WidgetEditor")) {

                    for (Widgets::WidgetBase *w : mWidgetManager->widgets()) {
                        if (ImGui::MenuItem(w->key().c_str(), nullptr, w->mVisible)) {
                            mWidgetManager->swapCurrentRoot(w);
                        }
                    }
                    if (ImGui::Button("Create Layout")) {
                        mWidgetManager->createTopLevel();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
        }
        ImGui::End();
    }

    std::string_view WidgetEditor::key() const
    {
        return "WidgetEditor";
    }

    void WidgetEditor::open(Resources::ResourceBase *res)
    {
        Widgets::WidgetLoader::Resource *widget = static_cast<Widgets::WidgetLoader::Resource *>(res);

        mFiles.try_emplace(widget, *this, widget);
    }

    void WidgetEditor::renderWidgetBorders(Widgets::WidgetBase *widget, Engine::Vector2i screenOffset, ImU32 color)
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        ImGuiIO &io = ImGui::GetIO();

        Vector3 absoluteSize = widget->getAbsoluteSize();
        Vector2 absolutePos = widget->getAbsolutePosition() + Vector2 { screenOffset };

        Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

        drawList->AddRect(ImVec2 { bounds.topLeft() } / io.DisplayFramebufferScale, ImVec2 { bounds.bottomRight() } / io.DisplayFramebufferScale, color);
    }

    Widgets::WidgetBase *WidgetEditor::handleManagerInteractions(Widgets::WidgetManager &manager, const ImVec2 &pos)
    {
        Widgets::WidgetBase *hoveredWidget = manager.hoveredWidget();

        WidgetSettings *hoveredSettings = nullptr;
        if (hoveredWidget) {
            Rect2i screenSpace = manager.getClientSpace();
            screenSpace.mTopLeft = { static_cast<int>(pos.x), static_cast<int>(pos.y) };

            ImGuiIO &io = ImGui::GetIO();

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                screenSpace.mTopLeft += manager.getScreenSpace().mTopLeft;

            renderWidgetBorders(hoveredWidget, screenSpace.mTopLeft, IM_COL32(127, 127, 127, 255));
        }

        return hoveredWidget;
    }

}
}
